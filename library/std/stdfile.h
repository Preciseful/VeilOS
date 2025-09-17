#pragma once

typedef long FileID;

FileID OpenFile(const char *path);
void CloseFile(FileID id);
unsigned long ReadFromFile(FileID id, void *buf, unsigned long size);
unsigned long WriteToFile(FileID id, void *buf, unsigned long size);
void SeekInFile(FileID id, unsigned long seek);