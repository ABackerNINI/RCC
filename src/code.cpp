#include "code.h"
#include "debug_fmt.h"
#include "rcc.h"

namespace rcc {

// Init the output cpp and bin paths.
// This requires the full_code to be generated first.
void RCCode::init_cpp_bin_paths() {
    if (!full_code_generated) {
        gen_full_code();
    }
    // the output cpp file and executable file's name
    const std::string filename = RCC::gen_first_hash_filename(settings, full_code);
    paths.get_src_bin_full_path(filename, cpp_path, bin_path);
}

// Check if the binary is cached and the content matches.
//* The file hash may collide, so we need to check the content as well.
bool RCCode::is_cached() {
    if (bin_path.exists()) {
        if (!full_code_generated) {
            gen_full_code();
        }

        const std::string code_old = cpp_path.read_file();
        if (code_old == full_code) {
            return true;
        }

        gpdebug(red_bold, "WARNING: hash collided but content does not match!\n");
        gpdebug("{}:\n{}", styled("Old Code", fg(color::red)), code_old);
        gpdebug("{}:\n{}", styled("New Code", fg(color::red)), full_code);
    }
    return false;
}

// Write the full code to the cpp file and compile it.
// This requires the full_code to be generated first.
bool RCCode::compile(bool silent) {
    if (!full_code_generated) {
        gen_full_code();
    }

    try {
        // Write c++ code to the cpp file
        cpp_path.write_file(full_code);
    } catch (std::exception &e) {
        gperror("Failed to write code to file: %s", e.what());
        return false;
    }

    const auto ts = fg(color::dodger_blue) | emphasis::bold;
    gpdebug(ts, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    gpdebug(fg(terminal_color::yellow) | emphasis::bold, "Compiling {} code\n", code_name);

    const auto time_begin = now();

    bool result = RCC::compile_file(settings, cpp_path, bin_path, cs, silent);

    if (result) {
        gpdebug("COMPILATION {} ({})", styled("OK", green_bold), code_name);
    } else {
        gpdebug("COMPILATION {} ({})", styled("FAILED", red_bold), code_name);
    }
    double duration = duration_ms(time_begin);
    gpdebug_ex(", {}: {:.2f} ms\n", styled("TIME", fg(terminal_color::yellow) | emphasis::bold),
               colored_duration(100, 600, duration));
    gpdebug(ts, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");

    return result;
}

// Run the binary executable, return the exit status of the executable, or 1 on error.
int RCCode::run_bin() {
    return RCC::run_bin(settings, cpp_path, bin_path);
}

// Generate the full code with the given code and settings.
void RCCode::gen_full_code() {
    full_code = cs.gen_code(paths.get_template_file_path(), settings.get_additional_includes(),
                            settings.get_above_main(), settings.get_functions(), code, identifier);
    full_code_generated = true;
}

} // namespace rcc
