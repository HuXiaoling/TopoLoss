/******************************************************************************
 * config.h           Compiler language support flags
 *
 * This file was generated automatically by the script bzconfig.
 * You should rerun bzconfig each time you switch compilers, install new
 * standard libraries, or change compiler versions.
 *
 */

 
#ifndef BZ_MS_BZCONFIG_H
#define BZ_MS_BZCONFIG_H
 
#define BZ_COMPILER_NAME   "msvc++.NET 2003"
#define BZ_COMPILER_OPTIONS "/O2 /Ot /Ob2 /G6 /arch:SSE /Zc:forScope"
#define BZ_OS_NAME         "WIN_NT-5.1 1.5.7(0.109/3/2)"
#define BZ_BZCONFIG_DATE   "Fri Apr 02 08:30:00  2004"
#define BZ_PLATFORM        "i686-pc-win32"

#if defined(WIN32) && defined(_MSC_VER) && defined(NDEBUG)
#pragma inline_depth( 255 )
#pragma inline_recursion( on )
#pragma auto_inline( on )
#endif
 
#define BZ_HAVE_NAMESPACES
#define BZ_HAVE_EXCEPTIONS
#define BZ_HAVE_RTTI
#define BZ_HAVE_MEMBER_CONSTANTS
#undef  BZ_HAVE_OLD_FOR_SCOPING
#define BZ_HAVE_EXPLICIT
#define BZ_HAVE_MUTABLE
#define BZ_HAVE_TYPENAME
#undef  BZ_HAVE_NCEG_RESTRICT
#undef  BZ_HAVE_NCEG_RESTRICT_EGCS
#define BZ_HAVE_BOOL
#define BZ_HAVE_CONST_CAST
#define BZ_HAVE_STATIC_CAST
#define BZ_HAVE_REINTERPRET_CAST
#define BZ_HAVE_DYNAMIC_CAST
#define BZ_HAVE_TEMPLATES
#define BZ_HAVE_PARTIAL_SPECIALIZATION
#define BZ_HAVE_PARTIAL_ORDERING
#define BZ_HAVE_DEFAULT_TEMPLATE_PARAMETERS
#define BZ_HAVE_MEMBER_TEMPLATES
#define BZ_HAVE_MEMBER_TEMPLATES_OUTSIDE_CLASS
#define BZ_HAVE_FULL_SPECIALIZATION_SYNTAX
#define BZ_HAVE_FUNCTION_NONTYPE_PARAMETERS
#define BZ_HAVE_TEMPLATE_QUALIFIED_BASE_CLASS
#define BZ_HAVE_TEMPLATE_QUALIFIED_RETURN_TYPE
#define BZ_HAVE_EXPLICIT_TEMPLATE_FUNCTION_QUALIFICATION
#define BZ_HAVE_TEMPLATES_AS_TEMPLATE_ARGUMENTS
#define BZ_HAVE_TEMPLATE_KEYWORD_QUALIFIER
#define BZ_HAVE_TEMPLATE_SCOPED_ARGUMENT_MATCHING
#define BZ_HAVE_TYPE_PROMOTION
#define BZ_USE_NUMTRAIT
#define BZ_HAVE_ENUM_COMPUTATIONS
#define BZ_HAVE_ENUM_COMPUTATIONS_WITH_CAST
#define BZ_HAVE_COMPLEX
#define BZ_HAVE_NUMERIC_LIMITS
#define BZ_HAVE_CLIMITS
#define BZ_HAVE_VALARRAY
#define BZ_HAVE_COMPLEX_FCNS
#define BZ_HAVE_COMPLEX_MATH1
#undef  BZ_HAVE_COMPLEX_MATH2
#undef  BZ_HAVE_IEEE_MATH
#undef  BZ_HAVE_SYSTEM_V_MATH
#define BZ_MATH_FN_IN_NAMESPACE_STD
#define BZ_HAVE_COMPLEX_MATH_IN_NAMESPACE_STD
#define BZ_HAVE_STD
#define BZ_HAVE_STL
#undef  BZ_HAVE_RUSAGE
 
#endif // BZ_MS_BZCONFIG_H
