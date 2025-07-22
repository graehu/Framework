#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace fw
{
   namespace filesystem
   {
      // todo: handle windows slashes.
      inline int makedirs(const char *path, mode_t mode = 0700)
      {
	 char temp[256];
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
   }
}

