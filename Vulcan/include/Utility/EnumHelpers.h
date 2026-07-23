#pragma once

#define ENUM_OPERATORS(ENUM, SPACE) \
	inline bool operator==(ENUM lhs, ENUM rhs){ return static_cast<SPACE>(lhs) == static_cast<SPACE>(rhs); } \
	inline bool operator!=(ENUM lhs, ENUM rhs){ return static_cast<SPACE>(lhs) != static_cast<SPACE>(rhs); } \
	inline ENUM operator|(ENUM lhs, ENUM rhs){ return static_cast<ENUM>(static_cast<SPACE>(lhs) | static_cast<SPACE>(rhs)); } \
	inline ENUM operator&(ENUM lhs, ENUM rhs){ return static_cast<ENUM>(static_cast<SPACE>(lhs) & static_cast<SPACE>(rhs)); } \
	inline ENUM operator^(ENUM lhs, ENUM rhs){ return static_cast<ENUM>(static_cast<SPACE>(lhs) ^ static_cast<SPACE>(rhs)); } \
	inline ENUM operator|=(ENUM lhs, ENUM rhs){ return static_cast<ENUM>(static_cast<SPACE>(lhs) | static_cast<SPACE>(rhs)); } \
	inline ENUM operator&=(ENUM lhs, ENUM rhs){ return static_cast<ENUM>(static_cast<SPACE>(lhs) & static_cast<SPACE>(rhs)); } \
	inline ENUM operator^=(ENUM lhs, ENUM rhs){ return static_cast<ENUM>(static_cast<SPACE>(lhs) ^ static_cast<SPACE>(rhs)); } \
	inline ENUM operator~(ENUM lhs){ return static_cast<ENUM>(~static_cast<SPACE>(lhs)); }