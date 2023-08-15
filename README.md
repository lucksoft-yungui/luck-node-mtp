# luck-node-mtp

`nodejs` cross-platform android device file management tool based on [libmtp](https://github.com/libmtp/libmtp). `MacOs`, `Windows`, and `Linux` platforms are supported.

English | [简体中文](./README-zh_CN.md)

```
npm install luck-node-mtp
```

## Simple example

simple use dmeo:
```javascript
const mtp = require('luck-node-mtp');

// Connect to the first found device.
mtp.connect();

// Download a file from the device to the local system.
mtp.download('data/com.ahyungui.android/db/upload.zip', '/Users/tmp/upload.zip', (send, total) => {
  console.log('progress', send, total);
});

// Release the device.
mtp.release();
```

## Try/catch example

Wrapping your connection in a try/catch block ensures that the device can be released.

```javascript
const mtp = require('luck-node-mtp');

mtp.connect();

try {
  mtp.download('data/com.ahyungui.android/db/upload.zip', '/Users/tmp/upload.zip', (send, total) => {
    console.log('progress', send, total);
  });
} catch(error) {
  console.error(error);
} finally {
  mtp.release();
}
```

## Detailed example

Connect to a specific device with its known vendor and product ids. Find its storage named 'DATA' then set that storage. Find a file named 'download.zip' then download that file to your local system.

```javascript
mtp.connect(0x061a, 0x1100);

// Get a list of storage partitions on the device.
// Search for a partition named 'DATA'.
const foundStorage = mtp.getCurrentDeviceStorageInfo().filter((storage) => storage.StorageDescription.toUpperCase() === 'DATA');

if (foundStorage.length === 1) {
  // Set the storage partition.
  mtp.setStorage(foundStorage[0].id);

  // Get a list of files and folders in the search directory on the device.
  // Search for a specific file named 'download.zip'.
  const foundObjects = mtp.getList('/Data/User/Download').filter((obj) => obj.type === 'FILE' && obj.name === 'download.zip');

  if (foundObjects.length === 1) {
    const source = foundObjects[0];
    const sourcePath = `/Data/User/Download/${source.name}`;
    const targetPath = `/Users/tmp/${source.name}`;

    mtp.download(sourcePath, targetPath, (send, total) => {
      // Print the progress percentage.
      console.log(`${(total / source.size) * 100}%`);
    });
  }
}
```

# Methods

## connect()

### Structure

bool connect(uint vid?, uint pid?)

### Description

Connect device

If the vendor and product id parameters are not set the first device that is found will be connected.

- @param vid: Vendor id
- @param pid: Product id
- @return Get `true` if the operation was successful

```javascript
const result = mtp.connect();
```

To access a certain device through `vendor id` and `product id`. You can get the device information through the [getDeviceInfo](#getdeviceinfo) method.

```javascript
const result = mtp.connect(vid, pid);
```

## release()

### Structure

bool release()

### Description

Release the currently connected device.

The device needs to be released after use otherwise an error will be thrown when connecting again.

- @return Get `true` if the operate was successful

```javascript
const result = mtp.release();
```

## getList()

### Structure

array getList(string parentPath)

### Description

Get a list of objects within a given mtp device parent path. A file tree can be build by using this method.

 - @param parentPath: Parent path. Use `/` to scan the root directory.
 - @return: File info array

```javascript
const result = mtp.getList('/data/com.ahyungui.android/db/');
```

Example output:

```javascript
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

## getDeviceInfo()

### Structure

array getDeviceInfo()

### Description

Returns a list of valid connected devices.

 - @return Device information array

```javascript
const result = mtp.getDeviceInfo();
```

Examples output:

```javascript
[
  {
    vendor: 'MediaTek Inc',
    vendor_id: 3725,
    product: 'Elephone P8000',
    product_id: 8221
  }
]
```

## upload()

### Structure

bool upload(string localFilePath, string targetFolderPath, string progressCallBackFunction?)

### Description

Upload local files to the device.

- @param localFilePath: Source file path
- @param targetFolderPath: Target device parent folder path
- @param progressCallBackFun: Callback function for upload progress
  (send, total) => {}
- @return Get `true` if the operation was successful

```javascript
mtp.upload('/Users/tmp/download.zip', 'data/com.ahyungui.android/db', (send, total) => {
  console.log('progress', send, total);
});
```

## del()

### Structure

bool del(string targetPath)

### Description

This function deletes a single file, track, playlist, folder or
any other object from the MTP device, identified by the object ID.

If you delete a folder there is no guarantee that the device will
really delete all the files that were in that folder. It is
expected that they will not be deleted and will turn up in object
listings its parent set to a non-existant object ID. The safe way
to do this is to recursively delete all files (and folders) contained
in the folder then the folder itself.

- @param targetPath: The destination address on the device to delete
- @return Get `true` if the operation was successful

```javascript
mtp.del('/data/com.ahyungui.android/db/download.zip');
```

## getObject()

### Structure

object getObject(string targetPath)

### Description

Obtain information about an object on the device.

- @param targetPath: File path on device
- @return Return object information

```javascript
const result = mtp.get('data/com.ahyungui.android/db/upload.zip');
```

Example output:

```javascript
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

## copy()

### Structure

bool copy(string sourcePath, string targetFolderPath)

### Description

The semantics of copying a folder are not defined in the MTP spec, but it
appears to do the right thing when tested. Devices that implement
this operation are rare.

Copying an object may take a significant amount of time.

MTP does not provide any kind of progress mechanism so the operation
will simply block for the duration.

- @param sourcePath: The path of the file on the device to be copied
- @param targetFolderPath: The destination path on the device
- @return Get `true` if the operation was successful

```javascript
mtp.copy('/data/com.ahyungui.android/db/download.zip', '/data/com.ahyungui.android/db/8057');
```

## move()

### Structure

bool move(string sourcePath, string targetFolderPath)

### Description

The function moves an object from one location on the device to another
location.

The semantics of moving a folder are not defined in the spec, but it
appears to do the right thing when tested. Devices that implement
this operation are rare.

Moving an object may take a significant amount of time,
particularly if being moved between storage locations. MTP does not provide
any kind of progress mechanism, so the operation will simply block
for the duration.

- @param sourcePath: The path of the file on the device to be moved
- @param targetFolderPath: The destination path on the device
- @return Get `true` if the operation was successful

```javascript
mtp.move('/data/com.ahyungui.android/db/download.zip', '/data/com.ahyungui.android/db/8057');
```

## setFileName()

### Structure

bool setFileName(string targetPath, string newName)

### Description

Set the file name of an object on the device.

- @param targetPath: Path to the file on the device
- @param newName: New file name
- @return Get `true` if the operation was successful

```javascript
mtp.setFileName('/data/com.ahyungui.android/db/download.zip', 'download1.zip');
```

## setFolderName()

### Structure

bool setFolderName(string targetPath, string newName)

### Description

Set the name of a folder on the device.

- @param targetPath: Folder path on device
- @param newName: New folder name
- @return Get `true` if the operation was successful

```javascript
mtp.setFolderName('/data/com.ahyungui.android/db/8057', '8067');
```

## createFolder()

### Structure

uint createFolder(string parentFolderPath, string newName)

### Description

Create a folder on the device.

- @param targetPath: The path to the parent folder into which the new folder will be created
- @param newName: Folder name
- @return id

```javascript
mtp.createFolder('/data/com.ahyungui.android/db', 'test_folder');
```

## getCurrentDeviceStorageInfo()

### Structure

array getCurrentDeviceStorageInfo()

### Description

Get the storage information of the currently connected device.

- @return Store information array

```javascript
mtp.getCurrentDeviceStorageInfo();
```

Example output:

```javascript
[
  {
    id: 65537,
    StorageDescription: 'DATA',
    VolumeIdentifier: ''
  }
]
```

## setStorage()

### Structure

bool setStorage(uint storageId)

### Description

Select the device storage for the currently connected device.

If there are multiple storages in the device you can first obtain the device storage list through [getCurrentDeviceStorageInfo](#getcurrentdevicestorageinfo).

After setting the storage all of the file management operations will be based on this storage.

If the storage is not set the device will select the first storage automatically when connecting.

- @param storageId: Storage id
- @return Get `true` if the operation was successful

```javascript
mtp.connect();

mtp.getCurrentDeviceStorageInfo();

mtp.setStorage(result[0].id);

mtp.release();
```

# Prebuild

The current version has prebuilt binary files for `darwin-x64` and `win32-x64` which means that users of these two operating systems can use them without recompiling.

Users of other operating systems need to build their own development environment to recompile. Refer to the [Development Environment](#development-environment) chapter.

You can help out by issuing a pull request against `prebuild` if you've compiled for other systems.

> The Windows platform needs to install the `WinUSB`, `libusb-win32`, or `libusbK` driver to work properly. Use `Zadig` to manage the drivers of each device. The way to manage the drivers through `Zadig` is as follows:

- Connect your device to the system
- List all `USB` devices via `Zadig` menu
- Then select the `MTP` device to replace the driver

  As shown in the picture:

  ![picture 1](assets/b904613e8a533b236657fd6c8379ac601be46eae019c5cb29e04cd35f4ff4fb2.png)

> When a mobile phone is connected you need to select the `file transfer (MTP)` mode on your phone to use it as a MTP device.

## Precompiled Continuous Integration

There is an example under the `Mac` platform in `install/after.js`. You can do some follow-up work after the precompiled file is generated, such as:

```javascript
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

`xcode` and `nodejs 14+` are required.

You also need to install the `libmtp` library:

```bash
brew install libmtp
```

## Windows

`Visual Studio`, `nodejs 14+`, and `python3` are required.

## Linux

You need to install `libmtp-dev`:

```bash
sudo apt-get install libmtp-dev
```

# Test

You can run the following command to test the files under the `test` directory:

```
// test test/test_connect.js
npm run test connect
```

# Known issues

- Under Windows the storage name obtained by the getCurrentDeviceStorageInfo method may contain Chinese language characters.

# Development reference link

- [node-addon-api](https://github.com/nodejs/node-addon-api)
- [prebuildify](https://www.npmjs.com/package/prebuildify)
- [libmtp](https://github.com/libmtp/libmtp)
- [libusb](https://github.com/libusb/libusb)
