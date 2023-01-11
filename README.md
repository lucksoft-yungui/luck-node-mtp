# luck-node-mtp

`nodejs` cross-platform android device file management tool, based on [libmtp](https://github.com/libmtp/libmtp). Support `MacOs`, `Windows`, `linux` platforms.

English | [简体中文](./README-zh_CN.md) 

## Simple Demo

install

```
npm i luck-node-mtp
```

simple use dmeo:
```
const mtp = require("luck-node-mtp");

// connect first device be found
mtp.connect();

// download from device to local
mtp.download("data/com.ahyungui.android/db/upload.zip","/Users/tmp/upload.zip",(send,total)=>{
        console.log("progress",send,total);
});

// release device
mtp.release();
```

## More reliable example
```
const mtp = require("luck-node-mtp");

// connect first device be found
mtp.connect();

try{
    // download from device to local
    mtp.download("data/com.ahyungui.android/db/upload.zip","/Users/tmp/upload.zip",(send,total)=>{
        console.log("progress",send,total);
    });
}
catch(e)
{
    console.error(e);
}
finally{
    // release device
    mtp.release();
}
```
This ensures that the device can be released.

# api

## connect

### Structure
bool connect(uint vid?, uint pid?)

### Description
Connect device

If parameters vendor id and product id not set,the first device found will be connected

- @param vid: Vendor id
- @param pid: Product id
- @return Get `true` if the operate was successful

```
result = mtp.connect();
```

To access a certain device through `vendor id` and `product id`, you can get device information through [getDeviceInfo](#getdeviceinfo) method.

```
result = mtp.connect(vid,pid);
```

## release

### Structure

bool release()

### Description

Release the currently connected device.

The device needs to be released after use, otherwise an error will be reported when connecting again.

- @return Get `true` if the operate was successful

```
result = mtp.release();
```

## getList

### Structure

array getList(string parentPath)

### Description

Get object list by a mtp device parent path.

A file tree can be build by this method.

 - @param parentPath: Parent path. The root address can be passed in the string `/`.
 - @return: File info array

```
result = mtp.getList("/data/com.ahyungui.android/db/");
console.log("objArr:",result);
```

Examples of output results are as follows:

```
[
  {
    name: '1',
    size: 0,
    type: 'FOLDER',
    id: 156,
    modificationdate: 1672041505,
    parent_id: 152,
    storage_id: 65537
  },
  {
    name: 'download.zip',
    size: 8893742,
    type: 'FILE',
    id: 158,
    modificationdate: 1673321627,
    parent_id: 152,
    storage_id: 65537
  }
]
```

## getDeviceInfo

### Structure

array getDeviceInfo()

### Description

If multiple devices are connected, the device list can be obtained through this method.

 - @return Device information array

```
result = mtp.getDeviceInfo();
console.log("deviceArr:",result);
```

Examples of output results are as follows:

```
[
  {
    vendor: 'MediaTek Inc',
    vendor_id: 3725,
    product: 'Elephone P8000',
    product_id: 8221
  }
]
```

## upload

### Structure

bool upload(string localFilePath, string targetFolderPath, string progressCallBackFun?)

### Description

Upload local files to the device.

- @param localFilePath: Local file path to upload
- @param targetFolderPath: Target device parent folder path
- @param progressCallBackFun: Callback function for upload progress
  (send, total)=>{}
- @return Get `true` if the operate was successful
 
```
result = mtp.upload("/Users/tmp/download.zip", "data/com.ahyungui.android/db",  (send, total) => {
                    console.log("progress", send, total);
});
```

## del

### Structure

bool del(string targetPath)

### Description

This function deletes a single file, track, playlist, folder or
any other object off the MTP device, identified by the object ID.
 
If you delete a folder, there is no guarantee that the device will
really delete all the files that were in that folder, rather it is
expected that they will not be deleted, and will turn up in object
listings with parent set to a non-existant object ID. The safe way
to do this is to recursively delete all files (and folders) contained
in the folder, then the folder itself.

- @param targetPath: The destination address on the device to delete
- @return Get `true` if the operate was successful

```
result = mtp.del("/data/com.ahyungui.android/db/download.zip");
```

## getObject

### Structure

object getObject(string targetPath)

### Description

Obtain object information in the device, including files, folders, etc.

- @param targetPath: File path on device
- @return Return object information

```
result = mtp.get("data/com.ahyungui.android/db/upload.zip");
```

Examples of output results are as follows:

```
{
  name: 'upload.zip',
  size: 8893742,
  type: 'FILE',
  id: 163,
  modificationdate: 1673149510,
  parent_id: 152,
  storage_id: 65537
}
```

## copy

### Structure

bool copy(string sourcePath, string targetFolderPath)

### Description

The semantics of copying a folder are not defined in the spec, but it
appears to do the right thing when tested (but devices that implement
this operation are rare).
 
Note that copying an object may take a significant amount of time.

MTP does not provide any kind of progress mechanism, so the operation
will simply block for the duration.

- @param sourcePath: The path of the file to be copied in the device
- @param targetFolderPath: parent folder path where file copy to
- @return Get `true` if the operate was successful

```
result =  mtp.copy("/data/com.ahyungui.android/db/download.zip","/data/com.ahyungui.android/db/8057");
```

## move

### Structure

bool move(string sourcePath, string targetFolderPath)

### Description

The function moves an object from one location on a device to another
location.

The semantics of moving a folder are not defined in the spec, but it
appears to do the right thing when tested (but devices that implement
this operation are rare).
 
Note that moving an object may take a significant amount of time,
particularly if being moved between storages. MTP does not provide
any kind of progress mechanism, so the operation will simply block
for the duration.

- @param sourcePath: The path of the file to be copied in the device
- @param targetFolderPath: parent folder path where file copy to
- @return Get `true` if the operate was successful

```
result =  mtp.move("/data/com.ahyungui.android/db/download.zip","/data/com.ahyungui.android/db/8057");
```

## setFileName

### Structure

bool setFileName(string targetPath, string newName)

### Description

Set the file name in device.

- @param targetPath: File path on device
- @param newName: New file name
- @return Get `true` if the operate was successful

```
result = mtp.setFileName("/data/com.ahyungui.android/db/download.zip", "download1.zip");
```

## setFolderName

### Structure

bool setFolderName(string targetPath, string newName)

### Description

Set the folder name.

- @param targetPath: Folder path on device
- @param newName: New folder name称
- @return Get `true` if the operate was successful

```
result = mtp.setFolderName("/data/com.ahyungui.android/db/8057", "8067");
```

## createFolder

### Structure

uint createFolder(string parentFolderPath, string newName)

### Description

Create a folder on the current MTP device.

- @param targetPath: The parent folder path of the created folder
- @param newName: Folder name
- @return 文件夹的id

```
result = mtp.createFolder("/data/com.ahyungui.android/db", "test_folder");
```

## getCurrentDeviceStorageInfo

### Structure

array getCurrentDeviceStorageInfo()

### Description

Get the storage information of the currently connected device.

- @return Store information array

```
result = mtp.getCurrentDeviceStorageInfo();
```

Examples of output results are as follows:

```
[ 
  { 
    id: 65537, 
    StorageDescription: '内部共享存储空间', 
    VolumeIdentifier: '' 
  } 
]
```

## setStorage

### Structure

bool setStorage(uint storageId)

### Description

Set the default device store for the currently connected device.

If there are multiple storages in the device, you can first obtain the device storage list through [getCurrentDeviceStorageInfo](#getcurrentdevicestorageinfo),

After setting the default storage, all file management operations will be based on this storage by default.

If the default storage is not set, the device will select the first storage as the default storage when connecting.

- @param storageId: Storage id
- @return Get `true` if the operate was successful

```
mtp.connect();

result = mtp.getCurrentDeviceStorageInfo();

console.log("objArr:",result);

result = mtp.setStorage(result[0].id);
    
mtp.release();
```

# Prebuild

The current version has prebuild the binary files of `darwin-x64` and `win32-x64`, which means that users of these two OS can use them directly without recompiling.

Users of other OS need to build their own development environment to recompile. For details, please refer to the [Development Environment](#development-environment) chapter.

At the same time, I also look forward to your friends to PR the `prebuild` files of other systems that you have precompiled into this project.

> Note: The Windows platform needs to install the `WinUSB`, `libusb-win32` or `libusbK` driver to work properly, it is recommended to use `Zadig` to
> Manage the drivers of each device. The way to manage the drivers through `Zadig` is as follows:

- Connect your mobile device to the system
- List all `USB` devices via `Zadig` menu
- Then select the `MTP` device to replace the driver- 然后选择`MTP`设备更换驱动
  
  As shown in the picture:

  ![picture 1](assets/b904613e8a533b236657fd6c8379ac601be46eae019c5cb29e04cd35f4ff4fb2.png)  

> Note: In addition, when the mobile phone is connected to the system, you need to select the `file transfer (MTP)` mode to use it.


## Precompiled Continuous Integration

There is an example under the `Mac` platform in `install/after.js`, you can do some follow-up work after the precompiled file is generated, such as:

```
if (os.platform() == "darwin") {
    // init share obj
    shell.exec('cp -rf /usr/local/opt/libmtp/lib/libmtp.9.dylib ' + _rootPath + '/prebuilds/darwin-x64/libmtp.dylib');
    shell.exec('cp -rf /usr/local/opt/libusb/lib/libusb-1.0.0.dylib ' + _rootPath + '/prebuilds/darwin-x64/libusb.dylib');
    shell.exec('install_name_tool -change /usr/local/opt/libmtp/lib/libmtp.9.dylib @loader_path/libmtp.dylib ' + _rootPath + '/prebuilds/darwin-x64/node.napi.node');
    shell.exec('install_name_tool -change /usr/local/opt/libusb/lib/libusb-1.0.0.dylib @loader_path/libusb.dylib ' + _rootPath + '/prebuilds/darwin-x64/libmtp.dylib');
    console.log('node.napi.node links:',shell.exec('otool -L '+_rootPath+'/prebuilds/darwin-x64/node.napi.node'));
    console.log('libmtp.dylib links:',shell.exec('otool -L '+_rootPath+'/prebuilds/darwin-x64/libmtp.dylib'));
}
```

# Development Environment

## Mac

Need to install `xcode`, `nodejs 14+` development environment.

In addition, you need to install the `libmtp` library:

```
brew install libmtp
```

## Windows

Need to install `Visual Studio`, `nodejs 14+`, `python3` development environment.

## Linux

Need to install `libmtp-dev`:

```
sudo apt-get install libmtp-dev
```

# Known issues

- Under Windows, the storage name obtained by the getCurrentDeviceStorageInfo method contains Chinese characters and may be garbled.

# Development reference link

- [node-addon-api](https://github.com/nodejs/node-addon-api)
- [prebuildify](https://www.npmjs.com/package/prebuildify)
- [libmtp](https://github.com/libmtp/libmtp)
- [libusb](https://github.com/libusb/libusb)