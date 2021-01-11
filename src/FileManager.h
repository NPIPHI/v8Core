//
// Created by 16182 on 12/14/2020.
//

#ifndef V8DEBUGGER_FILEMANAGER_H
#define V8DEBUGGER_FILEMANAGER_H

#include<jni.h>
#include<android/asset_manager.h>
#include<android/asset_manager_jni.h>
#include<string>

class FileManager {
public:
    FileManager(JNIEnv * env, jobject jAssetManager);
    std::string read_file(std::string file_name);
    ~FileManager();

private:
    JNIEnv * env;
    jobject jAssetManager;
    AAssetManager * assetManager;
};


#endif //V8DEBUGGER_FILEMANAGER_H
