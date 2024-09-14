// When --target=wasm32 in passed to clang, it will use the a port of lld
// for WebAssembly as a linker https://lld.llvm.org/WebAssembly.html

// The linker port understands the attribute "import_name". Functions
// definitions with the import_name attribute are JavaScript functions,
// supplied in the WASM imports object, in "env" (by default).
__attribute__((import_name("displayInBrowser")))
void display_in_browser(char* str, int str_len);

__attribute__((import_name("alloc")))
void* alloc(int size);

// Likewise, functions marked with the "export_name" attribute will be
// exported to JavaScript in the WASM instance exports object.
__attribute__((export_name("main")))
int main(void) {

    //
    // Display "Hello from C!" 100 times in the browser
    //

    int   line_len = 14;
    char* line     = "Hello from C!\n";

    int   msg_len = 100*line_len + 1;
    char* msg     = alloc(msg_len);
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < line_len; ++j) {
            msg[i*line_len + j] = line[j];
        }
    }

    display_in_browser(msg, msg_len);
    return 0;
}

