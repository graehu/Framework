#pragma once

namespace fw
{
   namespace zip
   {
      bool archive(const char *folder_path, const char *zip_file);
      bool load(const char *zip_file);
   }
}

