// binary-stdin.h
// Set stdin and/or stdout to binary mode.

#ifndef SMBASE_BINARY_STDIN_H
#define SMBASE_BINARY_STDIN_H


// Set an arbitrary file descriptor to binary.
//
// On Windows, binary mode means that CRLF <-> LF translation will not
// occur, and the EOF character (dec 26, hex 1A) will not be treated
// specially.  The mode affects reading and writing using POSIX
// 'read/write', C 'fread/fwrite', and C++ iostream read/write.
//
// On unix, this has no effect.
void setFileDescriptorToBinary(int fd);


// Set a specific file descriptor to binary.
void setStdinToBinary();
void setStdoutToBinary();
void setStderrToBinary();


#endif // SMBASE_BINARY_STDIN_H
