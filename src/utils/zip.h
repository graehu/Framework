#pragma once
#include "blob.h"

namespace fw
{
   namespace zip
   {
      bool begin_archive(const char* zip_file);
      bool add_file(const char* file_path, const char* zip_path);
      bool end_archive();
      bool begin_load(const char *zip_file);
      int entry_count();
      bool load_entry(int index, blob::bank* in_bank, blob::allocation* alloc);

      template<typename T> bool load_index(int index, blob::bank* in_bank, blob::asset<T>* out_asset)
      {
	 bool ret = load_entry(index, in_bank, (blob::allocation*)out_asset);
	 assert(((out_asset->len-sizeof(out_asset->head)) % sizeof(*out_asset->data)) == 0);
	 assert(out_asset->head.ver == blob::fourcc);
	 return ret;
      }
      template<typename T> bool load_index(int index, blob::bank* in_bank, blob::assetnc<T>* out_asset)
      {
	 bool ret = load_entry(index, in_bank, (blob::allocation*)out_asset);
	 assert(((out_asset->len-sizeof(out_asset->head)) % sizeof(*out_asset->data)) == 0);
	 assert(out_asset->head.ver == blob::fourcc);
	 return ret;
      }
      bool end_load();
      bool load(const char *zip_file);
   }
}

