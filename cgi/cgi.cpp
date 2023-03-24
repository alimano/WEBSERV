#include "cgi.hpp"

int check_extension2(std::string name)
{
    if (name.substr(name.find_last_of(".") + 1) == "php" || name.substr(name.find_last_of(".") + 1) == "py")
        return(1);
    else
        return(0);
}


cgi::cgi(std::string p)
{
    path = p;
    cgi_pid = -1;
    pid_status = 0;
    php = "cgi-bin/php-cgi";
    py = "/usr/local/bin/python3";
}

cgi::~cgi()
{

}

int cgi::get_cgi_pid()
{
    return(this->cgi_pid);
}

std::string cgi::random_name()
{
    char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    int len = 5;
    srand(time(0));
    std::string name;
    name.reserve(len);
    for (int i = 0; i < len; i++)
        name += letters[rand() % (sizeof(letters) - 1)];
    return(name);
}

std::string cgi::get_outfile_path()
{
    char buff[PATH_MAX];
    getcwd(buff, PATH_MAX);
    std::string outfile_path(buff);
    outfile_path.append("/");
    outfile_path.append(outname);
    return(outfile_path);
}

void cgi::fill_env()
{
    std::string s = "PATH_INFO=";
    s.append(path);
    env = new char* [2];
    env[0] = new char [11 + path.size()];
    strcpy(env[0], s.c_str());
    env[1] = NULL;
}

void cgi::exec_cgi(char **args, char **env, int fd)
{
    cgi_pid = fork();
    if (cgi_pid == -1)
    {
        throw(fork_error());
    }
    if (cgi_pid != 0) 
    {
        dup2(fd, 0);
        dup2(out_fd, 1);
        if (execve(args[0], args, NULL) == -1)
            exit(1);
    }
}

void cgi::wait_cgi()
{
    int s;
    int pid = waitpid(cgi_pid, &s, WNOHANG);
    if (pid == -1)
        pid_status = ERROR;
    else if (pid != 0)
    {
        if (WIFSIGNALED(pid_status)){
			pid_status = ERROR;
		} else {
			pid_status = DONE;
		}
    }
    if (pid_status == DONE || pid_status == ERROR){
		close(out_fd);
		close(in_fd);
	}
}

int cgi::check_extension(std::string str)
{
    if (str.substr(str.find_last_of(".") + 1) == "php")
        return(1);
    else if (str.substr(str.find_last_of(".") + 1) == "py")
        return(2);
    else
        return(0);
}

void cgi::fill_args()
{
    int ext;
    ext = check_extension(path);
    args = new char* [3];
    if (ext == 1)
    {
        args[0] = new char[php.size() + 1];
        strcpy(args[0], php.c_str());
    }
    else if (ext == 2)
    {
        args[0] = new char[py.size() + 1];
        strcpy(args[0], py.c_str());
    }
    else
    {
        throw(extension_error());
    }
    args[1] = new char[path.size() + 1];
    strcpy(args[1], path.c_str());
    args[2] = NULL;
}

void cgi::exec()
{
    fill_args();
    if (access(args[0], F_OK | X_OK))
        throw(cgi_open_error());

    outname = random_name();
    in_fd = open("php.php", O_RDONLY);
    out_fd = open(outname.c_str(), O_RDWR | O_CREAT, 0666);
    fill_env();
    exec_cgi(args, env, in_fd);
}