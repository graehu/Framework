#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "miniz.h"

#ifdef _WIN32
    #include <windows.h>
    #define PATH_SEP '\\'
#else
    #include <dirent.h>
    #include <unistd.h>
    #define PATH_SEP '/'
#endif

// Ensure ZIP uses forward slashes internally
static void make_zip_path(char *path)
{
   for (char *p = path; *p; p++)
   {
      if (*p == '\\') *p = '/';
   }
}

#ifdef _WIN32
void zip_add_folder_recursive(mz_zip_archive *zip, const char *folder_path, const char *base_path)
{
   char search_path[1024];
   WIN32_FIND_DATA fd;
   HANDLE hFind;

   snprintf(search_path, sizeof(search_path), "%s\\*", folder_path);
   hFind = FindFirstFile(search_path, &fd);
   if (hFind == INVALID_HANDLE_VALUE) return;

   do
   {
      if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
      {
	 continue;
      }

      char full_path[1024], zip_path[1024];
      snprintf(full_path, sizeof(full_path), "%s\\%s", folder_path, fd.cFileName);
      snprintf(zip_path, sizeof(zip_path), "%s/%s", base_path, fd.cFileName);
      make_zip_path(zip_path);

      if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
	 // Add folder entry to ZIP
	 char dir_entry[1024];
	 snprintf(dir_entry, sizeof(dir_entry), "%s/", zip_path);
	 mz_zip_writer_add_mem(zip, dir_entry, NULL, 0, MZ_BEST_COMPRESSION);

	 // Recurse into subdir
	 zip_add_folder_recursive(zip, full_path, zip_path);
      }
      else
      {
	 mz_zip_writer_add_file(zip, zip_path, full_path, NULL, 0, MZ_BEST_COMPRESSION);
      }
   } while (FindNextFile(hFind, &fd));

   FindClose(hFind);
}
#else
void zip_add_folder_recursive(mz_zip_archive *zip, const char *folder_path, const char *base_path)
{
   DIR *dir;
   struct dirent *ent;
   struct stat st;

   if ((dir = opendir(folder_path)) == NULL) return;

   while ((ent = readdir(dir)) != NULL)
   {
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
      {
	 continue;
      }

      char full_path[1024], zip_path[1024];
      snprintf(full_path, sizeof(full_path), "%s/%s", folder_path, ent->d_name);
      if(base_path[0])
      {
	 snprintf(zip_path, sizeof(zip_path), "%s/%s", base_path, ent->d_name);
      }
      else
      {
	 snprintf(zip_path, sizeof(zip_path), "%s", ent->d_name);
      }
      make_zip_path(zip_path);
      if (stat(full_path, &st) == -1) continue;

      if (S_ISDIR(st.st_mode))
      {
	 char dir_entry[1024];
	 snprintf(dir_entry, sizeof(dir_entry), "%s/", zip_path);
	 mz_zip_writer_add_mem(zip, dir_entry, NULL, 0, MZ_BEST_COMPRESSION);
	 zip_add_folder_recursive(zip, full_path, zip_path);
      }
      else if (S_ISREG(st.st_mode))
      {
	 mz_zip_writer_add_file(zip, zip_path, full_path, NULL, 0, MZ_BEST_COMPRESSION);
      }
   }
   closedir(dir);
}
#endif

int zip_folder(const char *folder_path, const char *zip_file)
{
   mz_zip_archive zip;
   memset(&zip, 0, sizeof(zip));

   if (!mz_zip_writer_init_file(&zip, zip_file, 0)) {
      fprintf(stderr, "Failed to initialize zip file: %s\n", zip_file);
      return 0;
   }

   zip_add_folder_recursive(&zip, folder_path, "");

   mz_zip_writer_finalize_archive(&zip);
   mz_zip_writer_end(&zip);
   return 1;
}
