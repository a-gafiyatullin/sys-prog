#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define SLEEP_TIME 2

typedef struct copy_task {
  char src_dir[NAME_MAX];
  char dst_dir[NAME_MAX];
  short copy_type; // 0 - for a regular files, 1 - for a directory
} copy_task;

void *copy(void *task);

void add_new_path_element(char *new_path, char *path_start, char *path_end);

int copy_file(struct copy_task *task);

int copy_directory(struct copy_task *task);

int main(int argc, char *argv[]) {

  if (argc != 3) {
    fprintf(stderr, "usage: cp src_dir dest_dir");
    pthread_exit(NULL);
  }

  struct stat info;
  if (lstat(argv[1], &info) < 0) {
    perror(argv[1]);
    pthread_exit(NULL);
  }
  struct copy_task *task= malloc(sizeof(struct copy_task));
  strcpy(task->src_dir, argv[1]);
  strcpy(task->dst_dir, argv[2]);
  if (S_ISREG(info.st_mode)) { // copy only directories and regular files
    task->copy_type = 0;
  } else if (S_ISDIR(info.st_mode)) {
    task->copy_type = 1;
  } else {
    fprintf(stderr, "cp: error of the copying %s to %s\n", argv[1], argv[2]);
    pthread_exit(NULL);
  }
  copy(task);

  pthread_exit(NULL);
}

int copy_file(struct copy_task *task) {
  char buffer[BUFSIZ];
  struct stat info;
  if (lstat(task->src_dir, &info) < 0) {
    perror(task->src_dir);
    pthread_exit(NULL);
  }
  int src_file = open(task->src_dir, O_RDONLY);
  while (src_file < 0 && errno == EMFILE) {
    sleep(SLEEP_TIME);
    src_file = open(task->src_dir, O_RDONLY);
  }
  if (src_file < 0) {
    perror(task->src_dir);
    pthread_exit(NULL);
  }
  int dst_file = open(task->dst_dir, O_RDWR | O_CREAT, info.st_mode);
  while (dst_file < 0 && errno == EMFILE) {
    sleep(SLEEP_TIME);
    dst_file = open(task->dst_dir, O_RDWR | O_CREAT, info.st_mode);
  }
  if (dst_file < 0) {
    perror(task->dst_dir);
    close(src_file);
    pthread_exit(NULL);
  }

  size_t len = 0;
  while ((len = read(src_file, buffer, BUFSIZ)) > 0) {
    if (write(dst_file, buffer, len) < 0) {
      perror("write");
      break;
    }
  }

  close(dst_file);
  close(src_file);
  free(task);
  pthread_exit(NULL);
}

int copy_directory(struct copy_task *task) {
  pthread_t threads[PTHREAD_KEYS_MAX];
  int i = 0;

  DIR *src_dir = opendir(task->src_dir);
  while(src_dir == NULL && errno == EMFILE) {
    sleep(SLEEP_TIME);
    src_dir = opendir(task->src_dir);
  }
  if (src_dir == NULL) {
    perror(task->src_dir);
    pthread_exit(NULL);
  }
  DIR *dst_dir = opendir(task->dst_dir);
  while (dst_dir == NULL && errno == EMFILE) {
    sleep(SLEEP_TIME);
    dst_dir = opendir(task->dst_dir);
  }
  if (dst_dir == NULL) {
    perror(task->dst_dir);
    closedir(src_dir);
    pthread_exit(NULL);
  }

  struct stat info;
  struct dirent *file;
  struct copy_task *new_task;
  while ((file = readdir(src_dir))) {
    if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0) {
      continue;
    }
    new_task = malloc(sizeof(copy_task));
    if(new_task == NULL) {
      perror("malloc");
      break;
    }
    add_new_path_element(new_task->src_dir, ((copy_task *)task)->src_dir,
                         file->d_name);
    add_new_path_element(new_task->dst_dir, ((copy_task *)task)->dst_dir,
                         file->d_name);
    if (lstat(new_task->src_dir, &info) < 0) {
      perror(new_task->src_dir);
      break;
    }
    if (S_ISREG(info.st_mode)) {
      new_task->copy_type = 0;
    } else if (S_ISDIR(info.st_mode)) {
      new_task->copy_type = 1;
      if (mkdir(new_task->dst_dir, info.st_mode) < 0) {
        perror(new_task->dst_dir);
        break;
      }
    }
    if (pthread_create(&threads[i], NULL, copy, new_task) < 0) {
      perror("pthread_create");
      break;
    }
    if (pthread_detach(threads[i]) < 0) {
      perror("pthread_detach");
      break;
    }
    i++;
  }
  closedir(src_dir);
  closedir(dst_dir);
  free(task);
  pthread_exit(NULL);
}

void *copy(void *task) {
  (((copy_task *)task)->copy_type == 0 ? copy_file(task)
                                               : copy_directory(task));
  pthread_exit(NULL);
}

void add_new_path_element(char *new_path, char *path_start, char *path_end) {
  strcpy(new_path, path_start);
  strcpy(new_path + strlen(path_start), "/");
  strcpy(new_path + strlen(path_start) + 1, path_end);
}