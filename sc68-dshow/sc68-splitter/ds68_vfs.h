/**
 * @ingroup   sc68_directshow
 * @file      ds68_vfs.h
 * @brief     class Sc68VFS.
 * @author    Benjamin Gerard
 * @date      2014/06
 */

/* Copyright (c) 1998-2015 Benjamin Gerard */

#pragma once

#include "vector"

/**
 * Implenents sc68 @ref lib_file68_vfs "virtual file system".
 * @ingroup sc68_directshow
 */
class Sc68VFS
{
protected:
  vfs68_t vfs;                          ///< sc68 vfs base
  bool m_opened;                        ///< Is opened ?
  bool m_eos;                           ///< Has reach end of stream ?
  bool m_err;                           ///< Has failed ?
  int m_spl;                    ///< Current sample buffer index
  int m_pos;                    ///< Position in current sample buffer
  HANDLE m_lock;                ///< lock mutex
  HANDLE m_event;               ///< wake up reader event

  /// Samples received so far.
  std::vector<IMediaSample *> m_ppSamples;
  /// Free resources.
  void Free();
  /// Lock the object.
  void Lock() { ::WaitForSingleObject(m_lock,INFINITE); }
  /// Unlock the object.
  void Unlock() { ::ReleaseMutex(m_lock); }
  /// Wait for an event to wake up.
  void WaitEvent() { ::WaitForSingleObject(m_event,INFINITE); }

public:
  /// Get the VFS.
  vfs68_t * GetVFS() { ASSERT((void*)this == (void*)&vfs); return &vfs; }

  Sc68VFS();                            ///< @nodoc
  ~Sc68VFS();                           ///< @nodoc

  int Open();                           ///< Open the virtual file.
  int Close();                          ///< Close the virtual file.
  int Length();                         ///< Get virtual file size.
  int Tell();                           ///< Get current position.
  int Seek(int off);                    ///< Set positiion.
  int Read(void *mem, int len);         ///< Read data from virtual file.
  void Destroy();                       ///< Destroy the VFS.

  int Push(IMediaSample *pSample);      ///< Push upstream data
  int PushEos();                        ///< Notify end of upstream
  int PushErr(HRESULT hr);              ///< Notify an error upstream

};
