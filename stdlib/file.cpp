#include "file.hpp"

using namespace veil;

unsigned char *File::GetContent() {}
unsigned long File::Size() {}
void File::Write(const char *buf, unsigned long size) {}
void File::WriteText(const char *buf) {}
void File::Delete() {}
void File::Rename(const char *new_name) {}

File *File::Open(const char *dir) {}
void File::Close(File *&file) {}
bool File::Exists(const char *dir) {}
File *File::Create(const char *dir) {}