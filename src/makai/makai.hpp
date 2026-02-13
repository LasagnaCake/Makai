#ifndef MAKAILIB_H
#define MAKAILIB_H

#ifdef MAKAILIB_DEBUG
#define CTL_CONSOLE_OUT
#define NDEBUG
#endif

#include "core/core.hpp"
#include "data/data.hpp"
#include "parser/parser.hpp"
#include "lexer/lexer.hpp"
#include "lang/lang.hpp"
#include "file/file.hpp"
#include "graph/graph.hpp"
#include "audio/audio.hpp"
#include "tool/tool.hpp"
#include "net/net.hpp"

#endif // MAKAILIB_H
