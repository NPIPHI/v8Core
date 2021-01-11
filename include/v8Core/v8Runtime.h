//
// Created by 16182 on 12/13/2020.
//

#ifndef V8DEBUGGER_V8RUNTIME_H
#define V8DEBUGGER_V8RUNTIME_H

#include<v8.h>
#include<libplatform/libplatform.h>
#include<mutex>
#include<functional>
#include<v8Inspector/Inspector.h>

class v8Runtime {
public:
    v8Runtime(std::shared_ptr<v8::Platform> platform);

    /*
     * initializes v8
     * returns the v8::Platform
     */
    static std::shared_ptr<v8::Platform> initV8();

    /*
     * run the event loop until it is empty
     * this method acquires the necessary locks
     */
    void run_tasks_loop();

    /*
     * the inspector checks for queued messages and executes them
     * this method acquires the necessary locks
     */
    void run_inspector();

    /*
     * disposes the current isolate
     */
    void dispose();

    /*
     * runs a single task in the event loop
     * this method does not acquire the necessary locks
     */
    bool pump_message_loop();

    /*
     * resets the global context to its starting state
     * this does not delete the old context, it will be garbage collected when there are no more references
     */
    void reset_global_context();

    /*
     * returns weather the runtime is paused from hitting a breakpoint in the inspector
     * if the inspector is not attached this method always returns false
     */
    [[nodiscard]] bool paused() const;

    /*
     * returns the current "global" context
     */
    [[nodiscard]] v8::Persistent<v8::Context> * context();

    /*
     * returns a lock guard on the runtime's mutex
     */
    [[nodiscard]] std::lock_guard<std::mutex> get_lock();

    /*
     * posts a non nestable task to the event loop
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
     * attaches an inspector to the current global context
     * any files added before attaching the inspector will not appear in the debugger
     */
    void attach_inspector();

    /*
     * starts the inspector on the port specified
     */
    void start_inspector(int port);

    std::shared_ptr<v8::Platform> platform;
    v8::Isolate* isolate;

private:
    v8::Persistent<v8::Context> base_context;
    std::unique_ptr<Inspector> inspector;
    std::mutex mut;
};


#endif //V8DEBUGGER_V8RUNTIME_H
