//
// Created by 16182 on 12/13/2020.
//

#ifndef V8DEBUGGER_V8RUNTIME_H
#define V8DEBUGGER_V8RUNTIME_H

#include<v8.h>
#include<libplatform/libplatform.h>
#include<mutex>
#include<functional>

#if USE_V8_INSPECTOR
#include<v8Inspector/Inspector.h>
#endif

class v8Runtime {
public:
    /*
     * set_context_globals is called during construction and every time the context is reset
     * use set_context_globals to set builtin functions like setTimeout or performance.now()
     */
    v8Runtime(std::shared_ptr<v8::Platform> platform,
              std::function<void(v8Runtime *)> set_context_globals = [](auto){});

    ~v8Runtime();

    /*
     * initializes v8
     * returns the v8::Platform
     */
    static std::shared_ptr<v8::Platform> initV8();

    /*
     * run the event loop until it is empty
     */
    void run_tasks_loop();

    /*
     * runs a single task in the event loop
     */
    bool pump_message_loop();

    /*
     * resets the global context to its starting state
     * the old context will remain valid until it is garbage collected
     */
    void reset_global_context();

    /*
     * returns the current "global" context
     */
    [[nodiscard]] inline v8::Persistent<v8::Context> * context() {
        return &base_context;
    };

    /*
     * returns the current isolate
     */

    inline v8::Isolate* isolate() const {
        return _isolate;
    };

    /*
     * posts a nestable task to the event loop
     * the task will be run in pump_message_loop or run_tasks_loop
     */
    void post_task(std::function<void()> &&func);

    /*
     * posts a delayed task to the event loop
     * the task is executed only after the required time has passed
     */
    void post_task_delayed(std::function<void()> &&func, int delay_in_milliseconds);

    /*
     * adds a task that executes a script to the event loop
     * this function does not synchronously execute the script
     * the file name is only for use in the debugger
     */
    void add_script(std::string script_text, std::string file_name);

    /*
     * executes a script synchronously
     */
    void execute_script(std::string_view script_text, std::string_view file_name);

#if USE_V8_INSPECTOR
    /*
     * the inspector checks for queued messages and executes them
     */
    void run_inspector();

    /*
     * starts the inspector on the port specified
     */
    void start_inspector(int port);

     /*
     * returns weather the runtime is paused from hitting a breakpoint in the inspector
     * if the inspector is not attached this method always returns false
     */
    [[nodiscard]] bool paused() const;

#endif

private:
    std::function<void(v8Runtime*)> set_context_globals;
    v8::Persistent<v8::Context> base_context;
    v8::Isolate* _isolate;
    std::shared_ptr<v8::Platform> platform;

#if USE_V8_INSPECTOR
    std::unique_ptr<Inspector> inspector;
#endif
};


#endif //V8DEBUGGER_V8RUNTIME_H
