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

	 // todo: the order of the files is arbitrary, we need this to be consistent.
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
	 static blob::bank* current_bank = &blob::miscbank;
	 static char* current_file = nullptr;
	 if (!mz_zip_reader_init_file(&zip, zip_filename, 0))
	 {
	    fprintf(stderr, "Could not open ZIP archive: %s\n", zip_filename);
	    return false;
	 }
	 int num_files = (int)mz_zip_reader_get_num_files(&zip);
	 auto bank_allocate = [](void*, size_t items, size_t size)
	 {
	    blob::allocation alloc = {{}, nullptr, size};
	    current_bank->allocate(alloc);
	    printf("[ALLOC] %s: %zu x %zu bytes = %zu bytes @ %p\n", current_file, items, size, items * size, (void*)alloc.data);
	    return (void*)alloc.data;
	 };
	 auto bank_free = [](void*, void* address)
	 {
	    printf("[FREE] %s: %p\n", current_file, address);
	    blob::allocation alloc = {{}, (char*)address, 0};
	    current_bank->free(alloc);
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
	       assert(false);
	       continue;
	    }
	    if (mz_zip_reader_is_file_a_directory(&zip, i))
	    {
	       if(strstr(stat.m_filename, "images") == stat.m_filename)
	       {
		  current_bank = &blob::imagebank; printf("images\n");
	       }
	       else if (strstr(stat.m_filename, "meshes") == stat.m_filename)
	       {
		  current_bank = &blob::meshbank; printf("meshes\n");
	       }
	       continue;
	    }
	    size_t size;
	    current_file = stat.m_filename;
	    void* file = mz_zip_reader_extract_to_heap(&zip, i, &size, 0);
	    printf("%d loaded: %s\n", i, stat.m_filename);
	    assert(file != nullptr);
	 }
	 current_file = (char*)"none";
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
      mz_zip_archive write_archive;
      mz_zip_archive read_archive;
      bool begin_archive(const char* zip_file)
      {
	 memset(&write_archive, 0, sizeof(write_archive));
	 if (!mz_zip_writer_init_file(&write_archive, zip_file, 0))
	 {
	    fprintf(stderr, "Failed to initialize zip file: %s\n", zip_file);
	    return false;
	 }
	 return true;
      }
      bool add_file(const char* file_path, const char* zip_path)
      {
	 printf("adding %s -> %s\n", file_path, zip_path);
	 mz_zip_writer_add_file(&write_archive, zip_path, file_path, NULL, 0, MZ_BEST_COMPRESSION);
	 return true;
      }
      bool end_archive()
      {
	 mz_zip_writer_finalize_archive(&write_archive);
	 mz_zip_writer_end(&write_archive);
	 return true;
      }
      bool begin_load(const char* zip_file)
      {
	 memset(&read_archive, 0, sizeof(read_archive));
	 if (!mz_zip_reader_init_file(&read_archive, zip_file, 0))
	 {
	    fprintf(stderr, "Could not open ZIP archive: %s\n", zip_file);
	    return false;
	 }
	 return true;
      }
      int entry_count()
      {
	 return (int)mz_zip_reader_get_num_files(&read_archive);
      }
      bool load_entry(int index, blob::bank* in_bank, blob::allocation* out_entry)
      {
	 static blob::bank* current_bank = nullptr;
	 current_bank = in_bank;
	 auto bank_allocate = [](void*, size_t items, size_t size)
	 {
	    blob::allocation alloc = {{}, nullptr, size};
	    current_bank->allocate(alloc);
	    printf("[ALLOC]  %zu x %zu bytes = %zu bytes @ %p\n", items, size, items * size, (void*)alloc.data);
	    return (void*)alloc.data;
	 };
	 // auto bank_free = [](void*, void* address)
	 // {
	 //    printf("[FREE]  %p\n", address);
	 //    blob::allocation alloc = {{}, (char*)address, 0};
	 //    current_bank->free(alloc);
	 // };
	 auto bank_free = [](void*, void* address){printf("[no_free]  %p\n", address);};
	 auto bank_reallocate = [](void*, void*, size_t, size_t)
	 {
	    assert(false);
	    return (void*)nullptr;
	 };
	 read_archive.m_pAlloc = bank_allocate;
	 read_archive.m_pFree = bank_free;
	 read_archive.m_pRealloc = bank_reallocate;
	 mz_zip_archive_file_stat stat;
	 mz_zip_reader_file_stat(&read_archive, index, &stat);
	 printf("name: %s\n", stat.m_filename);
	 out_entry->data = (const char*)mz_zip_reader_extract_to_heap(&read_archive, index, &out_entry->len, 0);
	 return out_entry->data != nullptr;
      }
      bool end_load()
      {
	 auto bank_allocate = [](void*, size_t, size_t) {return (void*)nullptr;};
	 auto bank_free = [](void*, void* address){printf("[no_free]  %p\n", address);};
	 auto bank_reallocate = [](void*, void*, size_t, size_t){return (void*)nullptr;};
	 read_archive.m_pAlloc = bank_allocate;
	 read_archive.m_pFree = bank_free;
	 read_archive.m_pRealloc = bank_reallocate;
	 mz_zip_reader_end(&read_archive);
	 return true;
      }
      bool load(const char* zip_filename)
      {
	 return read_all_files_from_zip(zip_filename);
      }
   }
} // namespace fw
