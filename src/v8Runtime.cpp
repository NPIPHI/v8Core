//
// Created by 16182 on 12/13/2020.
//

#include "../include/v8Core/v8Runtime.h"
#include "log.h"

using namespace std::chrono_literals;

static std::string getExceptionMessage(v8::Local<v8::Context> & context, const v8::Local<v8::Value> &exception) {
    v8::Local<v8::Object> error = v8::Local<v8::Object>::Cast(exception);
    v8::String::Utf8Value exceptionMessage(context->GetIsolate(), error->ToString(context).ToLocalChecked());
    return *exceptionMessage;
}

struct funcTask : v8::Task{
    explicit funcTask(std::function<void()> && func): func(std::move(func)){}
    std::function<void()> func;
    void Run() override {
        func();
    }
    ~funcTask() override = default;
};


std::shared_ptr<v8::Platform> v8Runtime::initV8() {
    static bool has_initilized_ICU = false;
    if(!has_initilized_ICU){
        v8::V8::InitializeICU();
        has_initilized_ICU = true;
    }
    auto platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();
    return platform;
}

v8Runtime::v8Runtime(std::shared_ptr<v8::Platform> platform,
                     std::function<void(v8Runtime *)> set_context_globals) {
    this->platform = std::move(platform);
    this->set_context_globals = std::move(set_context_globals);
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    isolate = v8::Isolate::New(create_params);
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope scope(isolate);
    reset_global_context();
#if USE_V8_INSPECTOR
    inspector = std::make_unique<Inspector>(isolate, &base_context, [=](){return pump_message_loop();});
#endif
}


v8Runtime::~v8Runtime() {
    isolate->Dispose();
}


std::lock_guard<std::mutex> v8Runtime::get_lock() {
    return std::lock_guard(mut);
}

void v8Runtime::run_tasks_loop() {
    auto lock = get_lock();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope scope(isolate);
    while(v8::platform::PumpMessageLoop(platform.get(), isolate)) {}
}

void v8Runtime::post_task(std::function<void()> && func) {
    platform->GetForegroundTaskRunner(isolate)->PostNonNestableTask(std::make_unique<funcTask>(std::move(func)));
}

void v8Runtime::post_task_delayed(std::function<void()> && func, int delay_in_milliseconds) {
    platform->GetForegroundTaskRunner(isolate)->PostNonNestableDelayedTask(std::make_unique<funcTask>(std::move(func)), double(delay_in_milliseconds)/1000);
}

v8::Persistent<v8::Context> * v8Runtime::context() {
    return &base_context;
}

void v8Runtime::add_script(std::string script_text, std::string file_name) {
    post_task([=](){
            v8::ScriptOrigin scriptOrigin = v8::ScriptOrigin(
                v8::String::NewFromUtf8(
                        isolate,
                        ("file://" + file_name).c_str(),
                        v8::NewStringType::kNormal
                ).ToLocalChecked()
            );
            auto local_context = base_context.Get(isolate);
            v8::Context::Scope scope(local_context);
            auto source = v8::String::NewFromUtf8(isolate, script_text.data()).ToLocalChecked();
            auto maybe_script = v8::Script::Compile(local_context, source, file_name.empty() ? nullptr : &scriptOrigin);
            if(maybe_script.IsEmpty()){
                LOG_E("Could not compile the script with filename: %s", file_name.c_str());
            } else {
                auto script = maybe_script.ToLocalChecked();
                v8::TryCatch try_catch(isolate);
                auto result = script->Run(local_context);
                if(try_catch.HasCaught()){
                    LOG_E("Could not execut script: %s", file_name.c_str());
                    LOG_E("Exception: %s", getExceptionMessage(local_context, try_catch.Exception()).c_str());
                }
            }
    });
}

bool v8Runtime::pump_message_loop() {
    return v8::platform::PumpMessageLoop(platform.get(), isolate);
}

void v8Runtime::reset_global_context() {
    auto context = v8::Context::New(isolate);
    v8::Context::Scope context_scope(context);
    base_context.Reset(isolate, context);
    set_context_globals(this);
#if USE_V8_INSPECTOR
    if(inspector) inspector->set_context(&base_context);
#endif
}

#if USE_V8_INSPECTOR

void v8Runtime::start_inspector(int port) {
    inspector->start_agent(port);
}


void v8Runtime::run_inspector() {
    if(inspector){
        auto lock = get_lock();
        v8::Locker locker(isolate);
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope scope(isolate);
        inspector->poll_messages();
    }
}


bool v8Runtime::paused() const {
    if(inspector) return inspector->paused();
    else return false;
}

#endif