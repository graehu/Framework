#include <cstddef>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "miniz.h"
#include "string.h"
#include "zip.h"
#include "blob.h"

#ifdef _WIN32
#include <windows.h>
#define PATH_SEP '\\'
#else
#include <dirent.h>
#include <unistd.h>
#define PATH_SEP '/'
#endif

namespace fw
{
   namespace zip
   {
      // Ensure ZIP uses forward slashes internally
      static void make_zip_path(char *path)
      {
	 for (char *p = path; *p; p++)
	 {
	    if (*p == '\\') *p = '/';
	 }
      }

#ifdef _WIN32
      void zip_add_folder_recursive(mz_zip_archive *in_zip, const char *folder_path, const char *base_path)
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
	       mz_zip_writer_add_mem(in_zip, dir_entry, NULL, 0, MZ_BEST_COMPRESSION);

	       // Recurse into subdir
	       zip_add_folder_recursive(in_zip, full_path, zip_path);
	    }
	    else
	    {
	       mz_zip_writer_add_file(in_zip, zip_path, full_path, NULL, 0, MZ_BEST_COMPRESSION);
	    }
	 } while (FindNextFile(hFind, &fd));

	 FindClose(hFind);
      }
#else
      void zip_add_folder_recursive(mz_zip_archive *in_zip, const char *folder_path, const char *base_path)
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
	       mz_zip_writer_add_mem(in_zip, dir_entry, NULL, 0, MZ_BEST_COMPRESSION);
	       zip_add_folder_recursive(in_zip, full_path, zip_path);
	    }
	    else if (S_ISREG(st.st_mode))
	    {
	       mz_zip_writer_add_file(in_zip, zip_path, full_path, NULL, 0, MZ_BEST_COMPRESSION);
	    }
	 }
	 closedir(dir);
      }
      bool read_all_files_from_zip(const char *zip_filename)
      {
	 mz_zip_archive zip;
	 memset(&zip, 0, sizeof(zip));
	 static blob::bank& current_bank = blob::miscbank;
	 if (!mz_zip_reader_init_file(&zip, zip_filename, 0))
	 {
	    fprintf(stderr, "Could not open ZIP archive: %s\n", zip_filename);
	    return false;
	 }
	 int num_files = (int)mz_zip_reader_get_num_files(&zip);
	 auto bank_allocate = [](void*, size_t items, size_t size)
	 {
	    blob::allocation alloc = {{}, nullptr, size};
	    current_bank.allocate(alloc);
	    printf("[ALLOC] %zu x %zu bytes = %zu bytes @ %p\n", items, size, items * size, (void*)alloc.data);
	    return (void*)alloc.data;
	 };
	 auto bank_free = [](void*, void* address)
	 {
	    printf("[FREE] %p\n", address);
	    blob::allocation alloc = {{}, (char*)address, 0};
	    current_bank.free(alloc);
	 };
	 auto bank_reallocate = [](void*, void*, size_t, size_t)
	 {
	    assert(false);
	    return (void*)nullptr;
	 };
	 
	 zip.m_pAlloc = bank_allocate;
	 zip.m_pFree = bank_free;
	 zip.m_pRealloc = bank_reallocate;
	 
	 for (int i = 0; i < num_files; i++)
	 {
	    mz_zip_archive_file_stat stat;
	    if (!mz_zip_reader_file_stat(&zip, i, &stat))
	    {
	       fprintf(stderr, "Could not get file stat for index %d\n", i);
	       continue;
	    }
	    if (mz_zip_reader_is_file_a_directory(&zip, i))
	    {
	       if(strstr(stat.m_filename, "images") == stat.m_filename)
	       {
		  current_bank = blob::imagebank; printf("images\n");
	       }
	       else if (strstr(stat.m_filename, "meshes") == stat.m_filename)
	       {
		  current_bank = blob::meshbank; printf("meshes\n");
	       }
	       continue;
	    }
	    size_t size;
	    void* file = mz_zip_reader_extract_to_heap(&zip, i, &size, 0);
	    assert(file != nullptr);
	 }
	 mz_zip_reader_end(&zip);
	 return true;
      }
#endif
   }
} // namespace fw

namespace fw
{
   namespace zip
   {
      bool archive(const char *folder_path, const char *zip_file)
      {
	 mz_zip_archive zip;
	 memset(&zip, 0, sizeof(zip));
	 if (!mz_zip_writer_init_file(&zip, zip_file, 0))
	 {
	    fprintf(stderr, "Failed to initialize zip file: %s\n", zip_file);
	    return false;
	 }
	 zip_add_folder_recursive(&zip, folder_path, "");
	 mz_zip_writer_finalize_archive(&zip);
	 mz_zip_writer_end(&zip);
	 return true;
      }
      bool load(const char* zip_filename)
      {
	 return read_all_files_from_zip(zip_filename);
      }
   }
} // namespace fw
