//
// Created by 16182 on 12/14/2020.
//

#include "FileManager.h"

FileManager::FileManager(JNIEnv *env, jobject jAssetManager) {
    this->jAssetManager = env->NewGlobalRef(jAssetManager);
    assetManager = AAssetManager_fromJava(env, jAssetManager);
}

std::string FileManager::read_file(std::string file_name) {
    auto asset = AAssetManager_open(assetManager, file_name.c_str(), AASSET_MODE_BUFFER);
    if(asset == nullptr){
        throw std::runtime_error("could not open " + file_name);
    }
    auto len = AAsset_getLength(asset);
    auto buff = AAsset_getBuffer(asset);
    std::string contents{(const char *) buff, (const char *) buff + len};
    AAsset_close(asset);
    return contents;
}


FileManager::~FileManager() {
    if(jAssetManager) env->DeleteGlobalRef(jAssetManager);
}