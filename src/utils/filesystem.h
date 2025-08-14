#pragma once

#include <cstddef>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "hasher.h"

namespace fw
{
   namespace filesystem
   {
      const unsigned int max_path_len = 256;
      // todo: handle windows slashes.
      inline int makedirs(const char *path, mode_t mode = 0700)
      {
	 char temp[max_path_len];
	 char *p = NULL;
	 size_t len;

	 // Copy path to a temporary buffer
	 snprintf(temp, sizeof(temp), "%s", path);
	 len = strlen(temp);

	 // Remove trailing slash if present
	 if (temp[len - 1] == '/') { temp[len - 1] = '\0'; }

	 // Iterate over each part of the path
	 for (p = temp + 1; *p; p++)
	 {
	    if (*p == '/')
	    {
	       struct stat st = {};
	       *p = '\0';
	       if (stat(temp, &st) == -1) { mkdir(temp, 0700); }
	       *p = '/';
	    }
	 }

	 // Make the final director
	 struct stat st = {};
	 if (stat(temp, &st) == -1) { mkdir(temp, mode); }
	 return 0;
      }
      inline int countdirs(const char *path)
      {
	 DIR *dir;
	 struct dirent *entry;
	 struct stat statbuf;
	 char fullpath[max_path_len];
	 int count = 0;

	 dir = opendir(path);
	 if (!dir)
	 {
	    return 0;
	 }

	 while ((entry = readdir(dir)) != NULL)
	 {
	    // Skip . and ..
	    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
	    {
	       continue;
	    }

	    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

	    if (stat(fullpath, &statbuf) == 0)
	    {
	       if (S_ISDIR(statbuf.st_mode))
	       {
		  count++;
	       }
	    }
	 }

	 closedir(dir);
	 return count;
      }
      inline size_t get_modtime(const char* path)
      {
	 struct stat attr;
	 if (stat(path, &attr) == 0) {
	    return (size_t)attr.st_mtime;  // Last modification time
	 }
	 return 0;
      }
      inline bool exists(const char* path)
      {
	 struct stat buffer;
	 return (stat(path, &buffer) == 0);
      }
      inline hash::u32 filehash(const char* in_file)
      {
	 FILE* file = fopen(in_file, "rb");
	 if (file == nullptr) { return 0; }
	 fseek(file, 0, SEEK_END);
	 size_t size = ftell(file);
	 fseek(file, 0, SEEK_SET);
	 char* alloc = new char[size];
	 fread((char*)alloc, size, 1, file);
	 hash::u32 out = hash::hash_buffer(alloc, size);
	 fclose(file);
	 delete [] alloc;
	 return out;
      }
      inline bool remove_file(const char* path)
      {
	 return remove(path) == 0;
      }
      inline bool remove_dir(const char* path)
      {
	 return rmdir(path) == 0;
      }
   }
}

