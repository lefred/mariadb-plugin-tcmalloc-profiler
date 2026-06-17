
#define MYSQL_SERVER

#include "mariadb.h"
#include "common.h"

#include <filesystem>
#include <limits.h>
#include <sys/wait.h>
#include <unistd.h>
#include <array>
#include <cerrno>
#include <fcntl.h>
#include <memory>
#include <sstream>
#include <system_error>

namespace fs= std::filesystem;

static bool ends_with(const std::string &value, const std::string &suffix)
{
  return value.size() >= suffix.size() &&
         value.compare(value.size() - suffix.size(), suffix.size(), suffix) ==
             0;
}

static bool is_dump_file_with_prefix(const std::string &filename,
                                     const std::string &base)
{
  const std::string dump_prefix= base + ".";
  return filename.rfind(dump_prefix, 0) == 0 && ends_with(filename, ".heap");
}

std::filesystem::path
get_last_file_with_prefix(const std::filesystem::path &directory,
                          const std::string &prefix)
{

  fs::path last_file;
  if (directory.empty())
  {
    return last_file;
  }
  for (const auto &entry : fs::directory_iterator(directory))
  {
    if (entry.is_regular_file() &&
        entry.path().filename().string().find(prefix) == 0)
    {
      if (last_file.empty() ||
          entry.path().filename().string() > last_file.filename().string())
      {
        last_file= entry.path();
      }
    }
  }

  return last_file;
}

bool has_dump_with_prefix(const std::string &prefix_path)
{
  fs::path p(prefix_path);
  fs::path dir= p.parent_path().empty() ? "." : p.parent_path();
  std::string base= p.filename().string();
  std::error_code ec;

  if (!fs::exists(dir, ec) || ec || !fs::is_directory(dir, ec) || ec)
    return false;

  fs::directory_iterator it(dir, ec);
  fs::directory_iterator end;
  if (ec)
    return false;

  for (; it != end; it.increment(ec))
  {
    if (ec)
      return false;

    if (!it->is_regular_file(ec))
    {
      ec.clear();
      continue;
    }

    std::string fname= it->path().filename().string();
    if (is_dump_file_with_prefix(fname, base))
      return true;
  }
  return false;
}

int remove_dump_files_with_prefix(const std::string &prefix_path,
                                  std::string *failed_path, int *error_code)
{
  fs::path p(prefix_path);
  fs::path dir= p.parent_path().empty() ? "." : p.parent_path();
  std::string base= p.filename().string();
  int removed= 0;
  std::error_code ec;
  auto fail= [&]() {
    if (failed_path && failed_path->empty())
      *failed_path= dir.string();
    if (error_code)
      *error_code= ec.value();
    return -1;
  };

  if (base.empty())
    return 0;

  if (!fs::exists(dir, ec))
  {
    if (ec)
      return fail();
    return 0;
  }

  if (!fs::is_directory(dir, ec))
  {
    if (ec)
      return fail();
    return 0;
  }

  {
    fs::directory_iterator it(dir, ec);
    fs::directory_iterator end;
    if (ec)
      return fail();

    for (; it != end; it.increment(ec))
    {
      if (ec)
        return fail();

      if (!it->is_regular_file(ec))
      {
        ec.clear();
        continue;
      }

      std::string fname= it->path().filename().string();
      if (!is_dump_file_with_prefix(fname, base))
        continue;

      fs::remove(it->path(), ec);
      if (ec)
      {
        if (failed_path)
          *failed_path= it->path().string();
        return fail();
      }
      ++removed;
    }
  }

  return removed;
}

extern std::string get_mariadb_server_binary()
{
  char buf[PATH_MAX];
  ssize_t len= readlink("/proc/self/exe", buf, sizeof(buf) - 1);
  if (len == -1)
  {
    return {};
  }
  buf[len]= '\0';
  return std::string(buf);
}

extern std::string exec_pprof(const std::vector<std::string> &argv)
{
  std::string result;
  int pipefd[2];

  if (argv.empty())
    return "can't run pprof";

  if (pipe(pipefd) != 0)
    return "can't run pprof";

  pid_t pid= fork();
  if (pid < 0)
  {
    close(pipefd[0]);
    close(pipefd[1]);
    return "can't run pprof";
  }

  if (pid == 0)
  {
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    int devnull= open("/dev/null", O_WRONLY);
    if (devnull >= 0)
    {
      dup2(devnull, 2);
      close(devnull);
    }
    close(pipefd[1]);

    std::vector<char *> exec_args;
    exec_args.reserve(argv.size() + 1);
    for (const std::string &arg : argv)
      exec_args.push_back(const_cast<char *>(arg.c_str()));
    exec_args.push_back(nullptr);

    execvp(exec_args[0], exec_args.data());
    _exit(127);
  }

  close(pipefd[1]);

  std::array<char, 4096> buffer;
  ssize_t bytes;
  while ((bytes= read(pipefd[0], buffer.data(), buffer.size())) > 0)
    result.append(buffer.data(), static_cast<size_t>(bytes));
  close(pipefd[0]);

  int status;
  while (waitpid(pid, &status, 0) < 0 && errno == EINTR)
  {
  }

  return result;
}

// Function to limit the string to X lines
std::string limit_lines(const std::string &input, size_t max_lines)
{
  std::istringstream stream(input);
  std::ostringstream limited_stream;
  std::string line;
  size_t count= 0;

  while (count < max_lines && std::getline(stream, line))
  {
    limited_stream << line << '\n';
    count++;
  }

  return limited_stream.str();
}
