#pragma once

/*! \defgroup core Core
 * Core library primitives.
 *
 * The `go::error` represents go's `error` type and acts in pretty much the same way as
 * go's interfaces. Users can return `go::error`s from functions and the callers can check
 * for errors, enriching them with additional context and propagating up the call stack, or
 * recovering from them.
 *
 * To detect specific error types users are advised to use `go::is_error` and `go::as_error`.
 * Although, users may still use `go::error_cast` directly to drop overhead for \ref wrapping.
 *
 * To format errors with automatic wrapping, users can use `go::errorf`.
 *
 * Refer to \ref predefined to see a list of already available error types.
 *
 * Users shall implement their own errors by implementing `go::error_interface`
 * and then wrapping the implementation into `go::error_of` and defining type alias for it:
 *
 * ```
 * struct error_my_data : public go::error_interface
 * {
 *     auto message() const override -> std::string
 *     {
 *         return "my error";
 *     }
 * };
 *
 * using error_my = go::error_of<error_my_data>;
 * ```
 * @{
 */
/*! @} */

/*! \defgroup wrapping Wrapping
 * Wrapping allows errors to enrich already existing errors with context,
 * while still allowing users to condition on the wrapped error types.
 *
 * For example:
 * ```
 * using error_parse = go::error_of<error_parse_data>;
 * using warn_parse = go::error_of<warn_parse_data>;
 *
 * struct line_diagnostic_data : public go::error_interface
 * {
 *     line_diagnostic_data(size_t lineNo, go::error err) :
 *         lineNo_(lineNo), err_(std::move(err)) {}
 *
 *     auto message() const override -> std::string
 *     {
 *         return fmt::format("at line {}: {}", lineNo_, err_.message());
 *     }
 *
 *     auto unwrap() const override -> go::error
 *     {
 *         return err_;
 *     }
 *
 * private:
 *     size_t lineNo_;
 *     go::error err_;
 * }
 *
 * using line_diagnostic = go::error_of<line_diagnostic_data>;
 *
 * auto parse(document doc) -> go::error
 * {
 *     auto errs = go::error();
 *     for (size_t lineNo = 0; lineNo < doc.line_count(); lineNo++)
 *     {
 *         // parse_line can return either error_parse or warn_parse
 *
 *         auto err = parse_line(doc.get_line(lineNo));
 *         if (err)
 *         {
 *             err = go::make_error<line_diagnostic>(lineNo, err);
 *             errs = go::append_error(errs, err);
 *         }
 *     }
 *
 *     return errs;
 * }
 *
 * // Parse document
 * auto errs = parse(document);
 *
 * std::cout << errs << std::endl;
 *
 * if (go::is_error<error_parse>(errs))
 * {
 *     // Do something if found errors
 * }
 * ```
 * @{
 */
/*! @} */

/*! \defgroup predefined Predefined errors
 * This library contains a number of predefined errors, so that users can start using
 * the library from the get go.
 * @{
 */
/*! @} */

#include <go/error.hpp>
#include <go/error_string.hpp>
#include <go/error_code.hpp>
#include <go/error_cast.hpp>
#include <go/errorf.hpp>
#include <go/wrap.hpp>
