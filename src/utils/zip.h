#pragma once
#include "blob.h"

namespace fw
{
   namespace zip
   {
      bool init();
      bool begin_archive(const char* zip_file);
      bool add_file(const char* file_path, const char* zip_path);
      bool end_archive();
      bool begin_load(const char *zip_file);
      int entry_count();
      bool load_entry(int index, blob::bank* in_bank, blob::allocation* alloc);
      template<typename T> bool load_index(int index, blob::bank* in_bank, const blob::asset<T>* out_asset)
      {
	 bool ret = load_entry(index, in_bank, (blob::allocation*)out_asset);
	 blob::convert<T>((blob::allocation*)out_asset);
	 return ret;
      }
      bool end_load();
   }
}

