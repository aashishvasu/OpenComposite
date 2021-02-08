//
// Created by ZNix on 8/02/2021.
//

#pragma once

void ZeroMemory(void* data, size_t len);
int strncpy_s(char* dest, size_t dest_size, char const* src, size_t max);
int strcpy_s(char* dest, size_t dest_size, char const* src);
