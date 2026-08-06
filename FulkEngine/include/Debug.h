#pragma once

#if _DEBUG
#define DEFINE_DEBUG_FUNCTION(NAME) void Dbg_##NAME();
#else
#define DEFINE_DEBUG_FUNCTION(NAME) void Dbg_##NAME(){}
#endif