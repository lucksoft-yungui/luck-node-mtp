#include <napi.h>
#include <stdlib.h>
#include <string.h>
#include <regex>
#include <vector>
#include <iostream>
#include <sys/stat.h>
#include "libmtp.h"
#include "utils.h"

using namespace std;

LIBMTP_mtpdevice_t *__device;
LIBMTP_raw_device_t *__rawdevices;
int __numrawdevices;
uint32_t __storageId;

/**
 * helper function to init a file obj
 *
 * @param file a pointer to the file.
 * @param fileObj a reference to the fileObj need init
 */
void initFileObj(LIBMTP_file_t *file, Napi::Object &fileObj)
{
  fileObj.Set("name", file->filename);
  fileObj.Set("size", file->filesize);
  if (file->filetype == LIBMTP_FILETYPE_FOLDER)
  {
    fileObj.Set("type", "FOLDER");
  }
  else
  {
    fileObj.Set("type", "FILE");
  }
  fileObj.Set("id", file->item_id);
  fileObj.Set("modificationdate", file->modificationdate);
  fileObj.Set("parent_id", file->parent_id);
  fileObj.Set("storage_id", file->storage_id);
}

/**
 * helper function to find file by path and parent
 *
 * @param device a pointer to the device current connected.
 * @param path a reference to the current find path
 * @param path a pointer to the current find parent folder id
 * @return a file struct
 */
LIBMTP_file_t *doFindFile(LIBMTP_mtpdevice_t *device, string &path, LIBMTP_file_t *parent)
{

  int parentId = LIBMTP_FILES_AND_FOLDERS_ROOT;

  if (parent)
    parentId = parent->item_id;

  LIBMTP_file_t *files;

  files = LIBMTP_Get_Files_And_Folders(device,
                                       __storageId,
                                       parentId);

  LIBMTP_file_t *file, *tmp;

  file = files;
  while (file != NULL)
  {
    if (file->filename == path)
      return file;
    tmp = file;
    file = file->next;
    LIBMTP_destroy_file_t(tmp);
  }

  return 0;
}

/**
 * helper function to find a file in mtp device by path
 *
 * @param device a pointer to the device current connected.
 * @param targetPath a reference to the target path
 * @return a file struct
 */
LIBMTP_file_t *findFile(LIBMTP_mtpdevice_t *device, const string &targetPath)
{

  LIBMTP_file_t *file = NULL;
  vector<string> subPaths = split(targetPath, "/");

  for (string path : subPaths)
  {
    // fprintf(stdout, "path:%s\n", path.c_str());
    file = doFindFile(device, path, file);
    if (!file)
      return NULL;
  }

  return file;
}

/**
 * download or upload porgress callback
 *
 * @param sent the number of bytes sent so far
 * @param total the total number of bytes to send
 * @param data a user-defined dereferencable pointer
 * @return if anything else than 0 is returned, the current transfer will be
 *         interrupted / cancelled.
 */
int progress(const uint64_t sent, const uint64_t total, void const *const data)
{
  if (data)
  {
    Napi::CallbackInfo *info = (Napi::CallbackInfo *)data;
    Napi::Env env = info->Env();
    Napi::Function processCallback;

    if (info->Length() >= 3)
      processCallback = (*info)[2].As<Napi::Function>();

    if (processCallback)
    {
      processCallback.Call(env.Global(), {Napi::Number::New(env, sent), Napi::Number::New(env, total)});
    }
  }
  return 0;
}

/**
 * get raw device array into global variable <code>__rawdevices</code> and <code>__numrawdevices</code>
 *
 * @param env napi env object
 */
void getRawDevice(Napi::Env env)
{
  LIBMTP_error_number_t err;

  err = LIBMTP_Detect_Raw_Devices(&__rawdevices, &__numrawdevices);
  switch (err)
  {
  case LIBMTP_ERROR_NO_DEVICE_ATTACHED:
    // fprintf(stdout, "   No raw devices found.\n");
    throw Napi::Error::New(env, "No raw devices found.");
  case LIBMTP_ERROR_CONNECTING:
    // fprintf(stderr, "Detect: There has been an error connecting. Exiting\n");
    throw Napi::Error::New(env, "Detect: There has been an error connecting. Exiting.");
  case LIBMTP_ERROR_MEMORY_ALLOCATION:
    // fprintf(stderr, "Detect: Encountered a Memory Allocation Error. Exiting\n");
    throw Napi::Error::New(env, "NDetect: Encountered a Memory Allocation Error. Exiting.");
  case LIBMTP_ERROR_NONE:
    break;
  case LIBMTP_ERROR_GENERAL:
  default:
    // fprintf(stderr, "Unknown connection error.\n");
    throw Napi::Error::New(env, "Unknown connection error.");
  }
}

/**
 * helper function to get rawdevice by vid and pid
 *
 * @param vid device vendor id
 * @param pid device product id
 * @return a pointer to the raw device for the search result
 */
LIBMTP_raw_device_t *findRawDevice(int vid, int pid)
{
  for (int i = 0; i < __numrawdevices; i++)
  {
    LIBMTP_raw_device_t *rawdev = &__rawdevices[i];
    if (rawdev->device_entry.vendor_id == vid && rawdev->device_entry.product_id == pid)
    {
      return rawdev;
    }
  }
  return NULL;
}

/**
 * helper functon to get device storage
 *
 * @param device device current connected
 * @param storageId storage id
 * @return a pointer to the device storage for the search result
 */
LIBMTP_devicestorage_t *findStorage(LIBMTP_mtpdevice_t *device, uint32_t storageId)
{
  LIBMTP_devicestorage_t *storage;
  for (storage = __device->storage; storage != 0; storage = storage->next)
  {
    if (storage->id == storageId)
    {
      return storage;
    }
  }
  return NULL;
}

/**
 * get device info
 *
 * @param info napi callback info
 * @return device info array
 */
Napi::Array getDeviceInfo(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  getRawDevice(env);

  Napi::Array re = Napi::Array::New(env, __numrawdevices);

  for (int i = 0; i < __numrawdevices; i++)
  {
    Napi::Object deviceObj = Napi::Object::New(env);
    LIBMTP_raw_device_t *rawdev = &__rawdevices[i];
    deviceObj.Set("vendor", rawdev->device_entry.vendor);
    deviceObj.Set("vendor_id", rawdev->device_entry.vendor_id);
    deviceObj.Set("product", rawdev->device_entry.product);
    deviceObj.Set("product_id", rawdev->device_entry.product_id);
    re[i] = deviceObj;
  }

  return re;
}

/**
 * get current connected device storage info
 *
 * @param info napi callback info
 * @return storage info array
 */
Napi::Array getCurrentDeviceStorageInfo(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (!__device)
  {
    throw Napi::Error::New(env, "Device not connected.");
  }

  vector<Napi::Object> storageArr;
  LIBMTP_devicestorage_t *storage;
  for (storage = __device->storage; storage != 0; storage = storage->next)
  {
    Napi::Object storageObj = Napi::Object::New(env);
    storageObj.Set("id", storage->id);

    Napi::Value storageDescription = env.Null();
    if (storage->StorageDescription)
    {
      storageDescription = Napi::String::New(env, storage->StorageDescription);
    }

    Napi::Value volumeIdentifier = env.Null();
    if (storage->VolumeIdentifier)
    {
      volumeIdentifier = Napi::String::New(env, storage->VolumeIdentifier);
    }

    storageObj.Set("StorageDescription", storageDescription);
    storageObj.Set("VolumeIdentifier", volumeIdentifier);
    storageArr.push_back(storageObj);
  }

  Napi::Array re = Napi::Array::New(env, storageArr.size());
  for (string::size_type i = 0; i < storageArr.size(); i++)
  {
    re[i] = storageArr[i];
  }

  return re;
}

/**
 * set current storage
 *
 * @param info napi callback info
 *             info[0] [uint32] storage id to set
 * @return true if the operate was successful
 */
Napi::Boolean setStorage(const Napi::CallbackInfo &info)
{

  Napi::Env env = info.Env();

  if (info.Length() < 1)
  {
    throw Napi::Error::New(env, "Wrong number of arguments");
  }

  if (!info[0].IsNumber())
  {
    throw Napi::TypeError::New(env, "Wrong arguments");
  }

  uint32_t storageId = info[0].As<Napi::Number>().Uint32Value();

  if (!findStorage(__device, storageId))
  {
    throw Napi::TypeError::New(env, "Can find storage by the id");
  }

  __storageId = storageId;

  return Napi::Boolean::New(env, true);
}

/**
 * connect device
 * if parameters vendor id and product id not set,the first device found will be connected
 *
 * @param info napi callback info
 *             info[0] [uint32] device vendor id
 *             info[1] [uint32] device product id
 * @return true if the operate was successful
 */
Napi::Boolean connectDevice(const Napi::CallbackInfo &info)
{

  Napi::Env env = info.Env();

  uint32_t vid = 0, pid = 0;

  if (info.Length() > 1)
  {
    vid = info[0].As<Napi::Number>().Uint32Value();
    pid = info[1].As<Napi::Number>().Uint32Value();
  }

  getRawDevice(env);

  LIBMTP_raw_device_t *rawdev = &__rawdevices[0];

  if (vid > 0 && pid > 0)
  {
    rawdev = findRawDevice(vid, pid);
  }

  if (!rawdev)
  {
    throw Napi::Error::New(env, "Device can not be find.");
  }

  __device = LIBMTP_Open_Raw_Device_Uncached(rawdev);

  __storageId = __device->storage->id;

  return Napi::Boolean::New(env, true);
}

/**
 * release current connect device
 *
 * @param info napi callback info
 * @return true if the operate was successful
 */
Napi::Boolean release(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (!__device)
  {
    throw Napi::Error::New(env, "Device not connected.");
  }

  LIBMTP_Release_Device(__device);
  __device = NULL;
  return Napi::Boolean::New(env, true);
}

/**
 * download file from device
 *
 * @param info napi callback info
               info[0] [string] the file path to be downloaded
               info[1] [string] the local file path save to
               info[2] [function] the progress callback function
               @see progress()
 * @return true if the operate was successful
 */
Napi::Boolean download(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 2)
  {
    throw Napi::Error::New(env, "Wrong number of arguments");
  }

  if (!info[0].IsString() || !info[1].IsString() || (info.Length() >= 3 && !info[2].IsFunction()))
  {
    throw Napi::TypeError::New(env, "Wrong arguments");
  }

  string sourceFilePath = info[0].As<Napi::String>().Utf8Value();
  string targetFilePath = info[1].As<Napi::String>().Utf8Value();

  sourceFilePath = formatMtpPath(sourceFilePath);

  if (!__device)
  {
    throw Napi::Error::New(env, "Device not connected.");
  }
  // int fileId = findFile(device, "data/com.ahyungui.android/db/upload.zip");
  LIBMTP_file_t *file = findFile(__device, sourceFilePath);

  if (!file)
  {
    throw Napi::Error::New(env, "Can not find the source file.");
  }

  if (LIBMTP_Get_File_To_File(__device, file->item_id, targetFilePath.c_str(), progress, &info) != 0)
  {
    throw Napi::Error::New(env, "Error getting file from MTP device.");
  }

  return Napi::Boolean::New(env, true);
}

/**
 * upload file to device
 *
 * @param info napi callback info
               info[0] [string] the local file path to be upload
               info[1] [string] the device folder path where to uplaod
               info[2] [function] the progress callback function
               @see progress()
 * @return true if the operate was successful
 */
Napi::Boolean upload(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 2)
  {
    throw Napi::Error::New(env, "Wrong number of arguments");
  }

  if (!info[0].IsString() || !info[1].IsString() || (info.Length() >= 3 && !info[2].IsFunction()))
  {
    throw Napi::TypeError::New(env, "Wrong arguments");
  }

  string sourceFilePath = info[0].As<Napi::String>().Utf8Value();
  string targetFolderPath = info[1].As<Napi::String>().Utf8Value();

  targetFolderPath = formatMtpPath(targetFolderPath);

  if (!__device)
  {
    throw Napi::Error::New(env, "Device not connected.");
  }

  LIBMTP_file_t *genfile;
  string filename;
  uint64_t filesize;
  struct stat sb;

  if (stat(sourceFilePath.c_str(), &sb) == -1)
  {
    throw Napi::Error::New(env, "Error to read file info.");
  }

  filesize = sb.st_size;
  filename = sourceFilePath.substr(sourceFilePath.find_last_of("/\\") + 1);

  LIBMTP_file_t *parent = findFile(__device, targetFolderPath);

  if (!parent)
  {
    throw Napi::Error::New(env, "Can not find the target parent folder id.");
  }

  genfile = LIBMTP_new_file_t();
  genfile->filesize = filesize;
  genfile->filename = strdup(filename.c_str());
  genfile->filetype = find_filetype(strdup(filename.c_str()));
  genfile->parent_id = parent->item_id;
  genfile->storage_id = __storageId;

  if (LIBMTP_Send_File_From_File(__device, sourceFilePath.c_str(), genfile, progress, &info) != 0)
  {
    LIBMTP_destroy_file_t(genfile);
    throw Napi::Error::New(env, "Error upload file to MTP device.");
  }

  LIBMTP_destroy_file_t(genfile);

  return Napi::Boolean::New(env, true);
}

/**
 * This function deletes a single file, track, playlist, folder or
 * any other object off the MTP device, identified by the object ID.
 *
 * If you delete a folder, there is no guarantee that the device will
 * really delete all the files that were in that folder, rather it is
 * expected that they will not be deleted, and will turn up in object
 * listings with parent set to a non-existant object ID. The safe way
 * to do this is to recursively delete all files (and folders) contained
 * in the folder, then the folder itself.
 *
 * @param info napi callback info
               info[0] [string] the file or folder path to be delete
 * @return true if the operate was successful
 */
Napi::Boolean del(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 1)
  {
    throw Napi::Error::New(env, "Wrong number of arguments");
  }

  if (!info[0].IsString())
  {
    throw Napi::TypeError::New(env, "Wrong arguments");
  }

  string targetPath = info[0].As<Napi::String>().Utf8Value();

  targetPath = formatMtpPath(targetPath);

  if (!__device)
  {
    throw Napi::Error::New(env, "Device not connected.");
  }

  LIBMTP_file_t *file = findFile(__device, targetPath);

  if (!file)
  {
    throw Napi::Error::New(env, "Can not find the target object.");
  }

  if (LIBMTP_Delete_Object(__device, file->item_id) != 0)
  {
    throw Napi::Error::New(env, "Error to delete target object.");
  }

  return Napi::Boolean::New(env, true);
}

/**
 * get object list by a mtp device parent path
 *
 * @param info napi callback info
               info[0] [string] parent folder path,the root can be use character / to express
 * @return return all file and folder object list in the parent folder
 */
Napi::Array getList(const Napi::CallbackInfo &info)
{

  Napi::Env env = info.Env();

  if (info.Length() < 1)
  {
    throw Napi::Error::New(env, "Wrong number of arguments");
  }

  if (!info[0].IsString())
  {
    throw Napi::TypeError::New(env, "Wrong arguments");
  }

  string parentPath = info[0].As<Napi::String>().Utf8Value();

  parentPath = formatMtpPath(parentPath);

  if (!__device)
  {
    throw Napi::Error::New(env, "Device not connected.");
  }

  int parentId = LIBMTP_FILES_AND_FOLDERS_ROOT;

  if (parentPath.length() > 0)
  {
    LIBMTP_file_t *parent = findFile(__device, parentPath);

    if (!parent)
    {
      throw Napi::Error::New(env, "Can not find the parent object.");
    }

    parentId = parent->item_id;
  }

  LIBMTP_file_t *files;

  files = LIBMTP_Get_Files_And_Folders(__device,
                                       __storageId,
                                       parentId);

  LIBMTP_file_t *file, *tmp;
  file = files;

  vector<Napi::Object> fileArr;
  while (file != NULL)
  {
    Napi::Object fileObj = Napi::Object::New(env);
    initFileObj(file, fileObj);
    fileArr.push_back(fileObj);
    tmp = file;
    file = file->next;
    LIBMTP_destroy_file_t(tmp);
  }

  Napi::Array re = Napi::Array::New(env, fileArr.size());
  for (string::size_type i = 0; i < fileArr.size(); i++)
  {
    re[i] = fileArr[i];
  }
  return re;
}

/**
 * get object from mtp device
 * @param info napi callback info
               info[0] [string] a file path in device
 * @return return file object find by the path
 */
Napi::Object getObject(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 1)
  {
    throw Napi::Error::New(env, "Wrong number of arguments");
  }

  if (!info[0].IsString())
  {
    throw Napi::TypeError::New(env, "Wrong arguments");
  }

  string targetPath = info[0].As<Napi::String>().Utf8Value();

  targetPath = formatMtpPath(targetPath);

  if (!__device)
  {
    throw Napi::Error::New(env, "Device not connected.");
  }

  LIBMTP_file_t *file = findFile(__device, targetPath);

  if (!file)
  {
    throw Napi::Error::New(env, "Can not find the target object.");
  }
  Napi::Object fileObj = Napi::Object::New(env);
  initFileObj(file, fileObj);
  return fileObj;
}

/**
 * The function copies an object from one location on a device to another
 * location.
 *
 * The semantics of copying a folder are not defined in the spec, but it
 * appears to do the right thing when tested (but devices that implement
 * this operation are rare).
 *
 * Note that copying an object may take a significant amount of time.
 * MTP does not provide any kind of progress mechanism, so the operation
 * will simply block for the duration.
 *
 * @param info napi callback info
               info[0] [string] the file or folder path to be copy
               info[1] [string] the parent folder path copy to
 * @return true if the operate was successful
 */
Napi::Boolean copyObject(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 2)
  {
    throw Napi::Error::New(env, "Wrong number of arguments");
  }

  if (!info[0].IsString() || !info[1].IsString())
  {
    throw Napi::TypeError::New(env, "Wrong arguments");
  }

  string sourcePath = info[0].As<Napi::String>().Utf8Value();
  string targetFolderPath = info[1].As<Napi::String>().Utf8Value();

  sourcePath = formatMtpPath(sourcePath);
  targetFolderPath = formatMtpPath(targetFolderPath);

  if (!__device)
  {
    throw Napi::Error::New(env, "Device not connected.");
  }

  LIBMTP_file_t *sourceFile = findFile(__device, sourcePath);

  if (!sourceFile)
  {
    throw Napi::Error::New(env, "Can not find the source file to copy.");
  }

  LIBMTP_file_t *parent = findFile(__device, targetFolderPath);

  if (!parent)
  {
    throw Napi::Error::New(env, "Can not find the target parent folder copy to.");
  }

  if (LIBMTP_Copy_Object(__device, sourceFile->item_id, __storageId, parent->item_id) != 0)
  {
    throw Napi::Error::New(env, "Error to copy file");
  }

  return Napi::Boolean::New(env, true);
}

/**
 * The function moves an object from one location on a device to another
 * location.
 *
 * The semantics of moving a folder are not defined in the spec, but it
 * appears to do the right thing when tested (but devices that implement
 * this operation are rare).
 *
 * Note that moving an object may take a significant amount of time,
 * particularly if being moved between storages. MTP does not provide
 * any kind of progress mechanism, so the operation will simply block
 * for the duration.
 *
 * @param info napi callback info
               info[0] [string] the file or folder path to be move
               info[1] [string] the parent folder path move to
 * @return true if the operate was successful
 */
Napi::Boolean moveObject(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 2)
  {
    throw Napi::Error::New(env, "Wrong number of arguments");
  }

  if (!info[0].IsString() || !info[1].IsString())
  {
    throw Napi::TypeError::New(env, "Wrong arguments");
  }

  string sourcePath = info[0].As<Napi::String>().Utf8Value();
  string targetFolderPath = info[1].As<Napi::String>().Utf8Value();

  sourcePath = formatMtpPath(sourcePath);
  targetFolderPath = formatMtpPath(targetFolderPath);

  if (!__device)
  {
    throw Napi::Error::New(env, "Device not connected.");
  }

  LIBMTP_file_t *sourceFile = findFile(__device, sourcePath);

  if (!sourceFile)
  {
    throw Napi::Error::New(env, "Can not find the source file to move.");
  }

  LIBMTP_file_t *parent = findFile(__device, targetFolderPath);

  if (!parent)
  {
    throw Napi::Error::New(env, "Can not find the target parent folder move to.");
  }

  if (LIBMTP_Move_Object(__device, sourceFile->item_id, __storageId, parent->item_id) != 0)
  {
    throw Napi::Error::New(env, "Error to move file");
  }

  return Napi::Boolean::New(env, true);
}

/**
 * This function renames a single file.
 * This simply means that the PTP_OPC_ObjectFileName property
 * is updated, if this is supported by the device.
 *
 * @param info napi callback info
               info[0] [string] the file path to be rename
               info[1] [string] the new name for this file
 * @return true if the operate was successful
 */
Napi::Boolean setFileName(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 2)
  {
    throw Napi::Error::New(env, "Wrong number of arguments");
  }

  if (!info[0].IsString() || !info[1].IsString())
  {
    throw Napi::TypeError::New(env, "Wrong arguments");
  }

  string targetPath = info[0].As<Napi::String>().Utf8Value();
  string newName = info[1].As<Napi::String>().Utf8Value();

  targetPath = formatMtpPath(targetPath);

  if (!__device)
  {
    throw Napi::Error::New(env, "Device not connected.");
  }

  LIBMTP_file_t *file = findFile(__device, targetPath);

  if (!file)
  {
    throw Napi::Error::New(env, "Can not find the target object.");
  }

  if (LIBMTP_Set_File_Name(__device, file, newName.c_str()) != 0)
  {
    throw Napi::Error::New(env, "Error to rename file.");
  }

  return Napi::Boolean::New(env, true);
}

/**
 * This function renames a single folder.
 *
 * @param info napi callback info
               info[0] [string] the folder path to be rename
               info[1] [string] the new name for this folder
 * @return true if the operate was successful
 */
Napi::Boolean setFolderName(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 2)
  {
    throw Napi::Error::New(env, "Wrong number of arguments");
  }

  if (!info[0].IsString() || !info[1].IsString())
  {
    throw Napi::TypeError::New(env, "Wrong arguments");
  }

  string targetPath = info[0].As<Napi::String>().Utf8Value();
  string newName = info[1].As<Napi::String>().Utf8Value();

  targetPath = formatMtpPath(targetPath);

  if (!__device)
  {
    throw Napi::Error::New(env, "Device not connected.");
  }

  LIBMTP_file_t *file = findFile(__device, targetPath);

  if (!file)
  {
    throw Napi::Error::New(env, "Can not find the target object.");
  }

  LIBMTP_folder_t *folder = LIBMTP_new_folder_t();
  folder->folder_id = file->item_id;
  folder->name = file->filename;

  if (LIBMTP_Set_Folder_Name(__device, folder, newName.c_str()) != 0)
  {
    LIBMTP_destroy_folder_t(folder);
    throw Napi::Error::New(env, "Error to rename folder.");
  }

  LIBMTP_destroy_folder_t(folder);

  return Napi::Boolean::New(env, true);
}

/**
 * This create a folder on the current MTP device. The PTP name
 * for a folder is "association". The PTP/MTP devices does not
 * have an internal "folder" concept really, it contains a flat
 * list of all files and some file are "associations" that other
 * files and folders may refer to as its "parent".
 *
 * @param info napi callback info
 *             info[0] [string] the parent folder path where the new folder create in
 *             info[1] [string] name the name of the new folder. Note this can be modified
 *                              if the device does not support all the characters in the name.
 * @return true if the operate was successful
 */
Napi::Number createFolder(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 2)
  {
    throw Napi::Error::New(env, "Wrong number of arguments");
  }

  if (!info[0].IsString() || !info[1].IsString())
  {
    throw Napi::TypeError::New(env, "Wrong arguments");
  }

  string parentFolderPath = info[0].As<Napi::String>().Utf8Value();
  string newName = info[1].As<Napi::String>().Utf8Value();

  parentFolderPath = formatMtpPath(parentFolderPath);

  if (!__device)
  {
    throw Napi::Error::New(env, "Device not connected.");
  }

  LIBMTP_file_t *file = findFile(__device, parentFolderPath);

  if (!file)
  {
    throw Napi::Error::New(env, "Can not find the parent folder.");
  }

  if (file->filetype != LIBMTP_FILETYPE_FOLDER)
  {
    throw Napi::Error::New(env, "Parent is not a folder.");
  }

  int folderId = LIBMTP_Create_Folder(__device, strdup(newName.c_str()), file->item_id, __storageId);

  if (folderId == 0)
  {
    throw Napi::Error::New(env, "Error to create folder.");
  }

  return Napi::Number::New(env, folderId);
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  // multi require only call once
  LIBMTP_Init();
  exports.Set(Napi::String::New(env, "download"),
              Napi::Function::New(env, download));
  exports.Set(Napi::String::New(env, "connect"),
              Napi::Function::New(env, connectDevice));
  exports.Set(Napi::String::New(env, "release"),
              Napi::Function::New(env, release));
  exports.Set(Napi::String::New(env, "upload"),
              Napi::Function::New(env, upload));
  exports.Set(Napi::String::New(env, "del"),
              Napi::Function::New(env, del));
  exports.Set(Napi::String::New(env, "getList"),
              Napi::Function::New(env, getList));
  exports.Set(Napi::String::New(env, "get"),
              Napi::Function::New(env, getObject));
  exports.Set(Napi::String::New(env, "copy"),
              Napi::Function::New(env, copyObject));
  exports.Set(Napi::String::New(env, "move"),
              Napi::Function::New(env, moveObject));
  exports.Set(Napi::String::New(env, "setFileName"),
              Napi::Function::New(env, setFileName));
  exports.Set(Napi::String::New(env, "setFolderName"),
              Napi::Function::New(env, setFolderName));
  exports.Set(Napi::String::New(env, "createFolder"),
              Napi::Function::New(env, createFolder));
  exports.Set(Napi::String::New(env, "getDeviceInfo"),
              Napi::Function::New(env, getDeviceInfo));
  exports.Set(Napi::String::New(env, "getCurrentDeviceStorageInfo"),
              Napi::Function::New(env, getCurrentDeviceStorageInfo));
  exports.Set(Napi::String::New(env, "setStorage"),
              Napi::Function::New(env, setStorage));

  return exports;
}

NODE_API_MODULE(addon, Init)
