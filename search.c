#include <dirent.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int max_depth = -1;
int max_size = -1;
int show_details = 0;
char *search_str = NULL;
char *exec_cmd = NULL;
char *exec_once_cmd = NULL;
int exec_once = 0;

void get_mode_string(mode_t mode, char *buf) {
    const char chars[] = "rwxrwxrwx";
    for (size_t i = 0; i < 9; i++) {
        buf[i] = (mode & (1 << (8 - i))) ? chars[i] : '-';
    }
    buf[9] = '\0';
}

void search_dir(const char *name, int depth, char **matched_files, int *file_count) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name))) return;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            printf("%*s\033[32m[%s]\033[0m\n", depth * 2, "", entry->d_name);
            if (max_depth == -1 || depth < max_depth) {
                search_dir(path, depth + 1, matched_files, file_count);
            }
        } else {
            struct stat st;
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            stat(path, &st);
            int print = 1;
            if (max_size != -1 && st.st_size > max_size) print = 0;
            if (search_str != NULL && !strstr(entry->d_name, search_str)) print = 0;

            if (print) {
                if (show_details) {
                    char mode[10];
                    get_mode_string(st.st_mode, mode);
                    char timebuf[128];
                    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", localtime(&st.st_atime));
                    printf("%*s%-10s %-10ld %-20s %-20s\n", depth * 2, "", mode, st.st_size, timebuf, entry->d_name);
                } else {
                    printf("%*s%-10ld %-20s\n", depth * 2, "", st.st_size, entry->d_name);
                }

                // Store matched file path
                if (exec_once) {
                    matched_files[*file_count] = strdup(path);
                    (*file_count)++;
                }

                // Execute command for each file if specified
                if (exec_cmd != NULL) {
                    pid_t pid = fork();
                    if (pid == 0) { // Child process
                        char command[2048];
                        snprintf(command, sizeof(command), "%s '%s'", exec_cmd, path);
                        execl("/bin/sh", "sh", "-c", command, (char *)NULL);
                        perror("execl");
                        exit(1);
                    } else {
                        wait(NULL);
                    }
                }
            }
        }
    }
    closedir(dir);

    // Execute command once for all matching files if specified
    if (exec_once && exec_once_cmd != NULL && *file_count > 0) {
        pid_t pid = fork();
        if (pid == 0) { // Child process
            char command[2048] = {0};
            snprintf(command, sizeof(command), "%s", exec_once_cmd);

            // Append matched files to the command
            for (int i = 0; i < *file_count; i++) {
                snprintf(command + strlen(command), sizeof(command) - strlen(command), " '%s'", matched_files[i]);
            }

            execl("/bin/sh", "sh", "-c", command, (char *)NULL);
            perror("execl");
            exit(1);
        } else {
            wait(NULL);
        }
    }
}

int main(int argc, char **argv) {
    int opt;
    char *dir = ".";
    static struct option long_options[] = {
        {"size", required_argument, 0, 's'},
        {"find", required_argument, 0, 'f'},
        {"exec", required_argument, 0, 'e'},
        {"exec_once", required_argument, 0, 'E'},
        {0, 0, 0, 0}
    };

    // To store matched files
    char *matched_files[1024] = {0};
    int file_count = 0;

    while ((opt = getopt_long(argc, argv, "s:f:e:E:S", long_options, NULL)) != -1) {
        switch (opt) {
            case 's':
                max_size = atoi(optarg);
                break;
            case 'f':
                search_str = optarg;
                if (optind < argc && argv[optind][0] != '-') {
                    max_depth = atoi(argv[optind]);
                    optind++;
                }
                break;
            case 'S':
                show_details = 1;
                break;
            case 'e':
                exec_cmd = optarg; // Command to execute for each file
                break;
            case 'E':
                exec_once_cmd = optarg; // Command to execute once for all files
                exec_once = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-s size] [-f search_str [max_depth]] [-S] [-e exec_cmd] [-E exec_once_cmd]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    search_dir(dir, 0, matched_files, &file_count);

    // Free matched file paths
    for (int i = 0; i < file_count; i++) {
        free(matched_files[i]);
    }

    return 0;
}
