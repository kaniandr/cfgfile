//===--- cfgfile.c ----------- Configuration File -----------------*- C -*-===//
//
// This file implements a Simple configuration file parser. A configuration
// file has the follwing structure:
//
//===----------------------------------------------------------------------===//

#include <cfgfile.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CFG_BUFFER_SIZE 1024

static int isWhitespace(unsigned char c) {
  return isspace(c) && c != '\n';
}

int cfgopen(const char* filename, CFGFILE* fd) {
  FILE* in;
  char tmp[CFG_BUFFER_SIZE];
  int ch;
  int sizeOfString, ignoreTail;
  int emptyPrefixSize, finishPrefix;
  int emptySuffixSize;
  int numberOfLeftStrings;
  char* currentData;
  long beginOfFile;

  if ((in = fopen(filename, "rt")) == NULL)
    return 1;
  if ((beginOfFile = ftell(in)) < 0)
    return 1;

  fd->sizeOfString = 0;
  fd->numberOfStrings = 0;

  sizeOfString = 0;
  ignoreTail = 0;
  emptyPrefixSize = 0;
  finishPrefix = 0;
  emptySuffixSize = 0;
  while (fgets(tmp, sizeof(tmp), in) != NULL) {
    int i = 0;
    int tmpSize = strlen(tmp);
    if (!ignoreTail) {
      for (i = 0; i < tmpSize; ++i) {
        if (isWhitespace(tmp[i])) {
          if (!finishPrefix)
            ++emptyPrefixSize;
          else
            ++emptySuffixSize;
          continue;
        }
        finishPrefix = 1;
        if (tmp[i] == '#') {
          sizeOfString += i;
          ignoreTail = 1;
          break;
        }
        if (tmp[i] != '\n')
          emptySuffixSize = 0;
      }
    }
    if (tmp[tmpSize - 1] == '\n') {
      if (!ignoreTail)
        sizeOfString += tmpSize - 1;
      sizeOfString -= emptyPrefixSize;
      sizeOfString -= emptySuffixSize;
      if (sizeOfString > 0) {
        fd->sizeOfString =
          fd->sizeOfString > sizeOfString ? fd->sizeOfString : sizeOfString;
        ++fd->numberOfStrings;
        sizeOfString = 0;
      }
      ignoreTail = 0;
      finishPrefix = 0;
      emptyPrefixSize = 0;
      emptySuffixSize = 0;
    }
    else if (!ignoreTail) {
      sizeOfString += tmpSize;
    }
  }
  sizeOfString -= emptyPrefixSize;
  sizeOfString -= emptySuffixSize;
  if (sizeOfString > 0) {
    ++fd->numberOfStrings;
    fd->sizeOfString =
      fd->sizeOfString > sizeOfString ? fd->sizeOfString : sizeOfString;
    sizeOfString = 0;
  }

  if (fseek(in, beginOfFile, SEEK_SET) != 0)
    return 1;

  ++fd->sizeOfString;
  if (fd->numberOfStrings == 0) {
    fd->data = NULL;
    return 0;
  }
  fd->data = (char*)malloc(fd->sizeOfString * fd->numberOfStrings);

  numberOfLeftStrings = fd->numberOfStrings;
  currentData = fd->data;
  while ((ch = fgetc(in)) != EOF && (isWhitespace(ch) || ch == '\n'));
  if (ch != EOF)
    currentData[0] = ch;
  while (numberOfLeftStrings > 0 &&
    fgets(currentData + 1, fd->sizeOfString, in) != NULL) {
    int i = 0;
    int emptySuffixBegin = 0;
    int currentDataSize = strlen(currentData);
    if (currentData[currentDataSize - 1] != '\n') {
      char tmp[100];
      while (fgets(tmp, 100, in) != NULL && tmp[strlen(tmp) - 1] != '\n');
    } else {
      currentData[--currentDataSize] = 0;
    }
    if (currentDataSize == 0 || currentData[0] == '#') {
      while ((ch = fgetc(in)) != EOF && (isWhitespace(ch) || ch == '\n'));
      if (ch != EOF)
        currentData[0] = ch;
      continue;
    }
    emptySuffixBegin = currentDataSize;
    for (i = 0; i < currentDataSize; ++i) {
      if (currentData[i] == '#') {
        currentData[i] = 0;
        break;
      }
      if (isWhitespace(currentData[i])) {
        if (emptySuffixBegin == currentDataSize)
          emptySuffixBegin = i;
      } else {
        emptySuffixBegin = currentDataSize;
      }
    }
    currentData[emptySuffixBegin] = 0;
    currentData += fd->sizeOfString;
    --numberOfLeftStrings;
    while ((ch = fgetc(in)) != EOF && (isWhitespace(ch) || ch == '\n'));
    if (ch != EOF)
      currentData[0] = ch;
  }
  fclose(in);
  return 0;
}

int cfgclose(CFGFILE *fd) {
  if (fd == NULL)
    return 0;
  free(fd->data);
  return 0;
}

static char *cfgfindid(CFGFILE *fd, const char *id) {
  char *s = NULL;
  int idSize = strlen(id);
  int i;
  for(i= 0; i < fd->numberOfStrings; ++i){
    if ((s=strstr(&fd->data[i * fd->sizeOfString], id)) != NULL) {
      if ((isWhitespace(s[idSize]) || s[idSize]== ':') &&
           s == &fd->data[i * fd->sizeOfString])
       return  &fd->data[i*fd->sizeOfString];
    }
  }
  return NULL;
}

int cfgfindi(CFGFILE *fd, const char *id, int *value) {
  char *s = cfgfindid(fd, id);
  if (s == NULL)
    return 1;
  if (sscanf(s + strlen(id), "%d",value) != 1 )
    return 1;
  return 0;
}

int cfgfindf(CFGFILE *fd, const char *id, double *value) {
  char *s = cfgfindid(fd, id);
  if (s == NULL)
    return 1;
  if (sscanf(s + strlen(id), "%lf",value) != 1 )
    return 1;
  return 0;
}

int cfgfinds(CFGFILE *fd, const char *id, char **value) {
  char *s = cfgfindid(fd, id);
  if (s == NULL)
    return 1;
  *value = s + strlen(id);
  while (isWhitespace(**value))
    ++*value;
  if (**value == ':')
    ++*value;
  return 0;
}


static int cfgevals_impl(CFGFILE *fd,
    char **b, char **value, char **evalue, int *size) {
  int prefixsize = *b - *value;
  char *tail = *b + 2; // skip "${"
  for (; *tail != '\0'; ++tail) { 
    if (*tail == '}') {
      char *tmp = *b + 2;
      char *swaptmp = NULL;
      int tmpsize = 0;
      int suffixsize = *evalue - tail;
      int tailsize = tail - *b + 1;
      *tail = 0;
      if (cfgfinds(fd, tmp, &tmp) != 0)
        return 1;
      tmpsize = strlen(tmp);
      if (prefixsize + suffixsize + tmpsize > *size) {
        char *newvalue = (char *)realloc(*value,
          prefixsize + suffixsize + tmpsize + CFG_BUFFER_SIZE);
        if (newvalue == NULL)
          return 1;
        *value = newvalue;
        *b = *value + prefixsize;
        tail = *b + tailsize - 1;
        *evalue = tail + suffixsize;
        *size =
          prefixsize + suffixsize + tmpsize + CFG_BUFFER_SIZE;
      }
      if (tailsize != tmpsize) {
        swaptmp = (char *) malloc (suffixsize);
        if (swaptmp == NULL)
          return 1;
        memcpy(swaptmp, tail + 1, suffixsize);
        memcpy(*b + tmpsize, swaptmp, suffixsize);
        free(swaptmp);
      }
      memcpy(*b, tmp, tmpsize);
      --(*b);
      *evalue = *b + tmpsize + suffixsize;
      return 0;
    } 
    if (*tail != '$' || *(++tail) != '{')
      continue;
    --tail;
    if (cfgevals_impl(fd, &tail, value, evalue, size) != 0)
      return 1;
    *b = *value + prefixsize;
  }
  return 1;
}

int cfgevals(CFGFILE *fd, const char *id, char **value) {
  int size = 0;
  int extrasize = 0;
  int isnullvalue = (*value == NULL);
  int originsize = (!isnullvalue ? strlen(*value) : 0);
  int ssize = 0;
  char *newvalue = NULL;
  char *s = cfgfindid(fd, id);
  char *tail = NULL;
  char *evalue = NULL;
  if (s == NULL)
    return 1;
  s = s + strlen(id);
  while (isWhitespace(*s))
    ++s;
  if (*s == ':')
    ++s;

  ssize = strlen(s);
  tail = strchr(s, '$');
  extrasize = ((tail == NULL || *(tail + 1) != '{') ? 0 : CFG_BUFFER_SIZE);
  size = originsize + ssize + extrasize + 1;
  newvalue = (char *) realloc(*value, size);
  if (newvalue == NULL)
    return 1;
  *value = newvalue;
  memcpy(*value + originsize, s, ssize + 1);
  if (extrasize == 0)
    return 0;
  evalue = *value + originsize + ssize;
  for (tail = *value + originsize + (tail - s); *tail != '\0'; ++tail) {
    if (*tail != '$' || *(++tail) != '{')
      continue;
    --tail;
    if (cfgevals_impl(fd, &tail, value, &evalue, &size) != 0) {
      if (isnullvalue) {
        free(*value);
        *value = NULL;
      } else {
        *value = (char *)realloc(*value, originsize + 1);
      }
      return 1;
    }
  }
  *value = (char *)realloc(*value, evalue - *value + 1);
  return 0;
}
