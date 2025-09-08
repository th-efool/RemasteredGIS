// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
// Define one log category per severity level
DECLARE_LOG_CATEGORY_EXTERN(LogGIS_Debug, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogGIS_Info, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogGIS_Warn, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogGIS_Error, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogGIS_Fatal, Log, All);


class REGIS_API GISErrorHandler
{
public:
	GISErrorHandler();
	~GISErrorHandler();
};



enum class FLoggerLevel : uint8
{
	Debug,
	Info,
	Warn,
	Error,
	Fatal
};

class FLogger
{
	public:
	static FLogger& Get()
	{
		static FLogger Instance;
		return Instance;
	}

	static inline bool bCrashOnFatal = true; // configurable switch
	
	template<typename T>
	static T Handle(
		const T& value,
		FLoggerLevel level,
		const char* expr,
		const char* file,
		int line)
	{
		
		if (IsInvalidValue(value))
		{
			LogWithLevel(level, FString::Printf(
				TEXT("[%s:%d] Invalid expression: %s"),
				ANSI_TO_TCHAR(FileNameFromPath(file)),
				line,
				ANSI_TO_TCHAR(expr)));
		}
		return value;
	}
	
	template<typename T>
	T HandleOr(const T& value, const T& fallback, FLoggerLevel level,
			   const char* expr, const char* file, int line)
	{
		if (IsInvalidValue(value))
		{
			LogWithLevel(level, FString::Printf(
				TEXT("[%s:%d] Invalid expression: %s"),
				ANSI_TO_TCHAR(FileNameFromPath(file)),
				line,
				ANSI_TO_TCHAR(expr)));
			return fallback;
		}
		return value;
	}

	template<typename T>
	static T HandleMsg(
		const T& value,
		FLoggerLevel level,
		const FString& customMsg,
		const char* file,
		int line)
	{
		if (IsInvalidValue(value))
		{
			LogWithLevel(level, FString::Printf(
				TEXT("[%s:%d] %s"),
				ANSI_TO_TCHAR(FileNameFromPath(file)),
				line,
				*customMsg));
		}
		return value;
	}

private:
	
	static void LogWithLevel(FLoggerLevel level, const FString& msg)
	{
		switch (level)
		{
		case FLoggerLevel::Debug:
			UE_LOG(LogGIS_Debug, Log, TEXT("%s"), *msg);break;
		case FLoggerLevel::Info:
			UE_LOG(LogGIS_Info, Log, TEXT("%s"), *msg); break;
		case FLoggerLevel::Warn:
			UE_LOG(LogGIS_Warn, Warning, TEXT("%s"), *msg); break;
		case FLoggerLevel::Error:
			UE_LOG(LogGIS_Error, Error, TEXT("%s"), *msg); break;
		case FLoggerLevel::Fatal:
			UE_LOG(LogGIS_Fatal, Fatal, TEXT("%s"), *msg); 
			if (bCrashOnFatal)
			{checkf(false, TEXT("%s"), *msg);}
			break;
			;
		}
	}

	// --- Generic fallback (assume valid) ---
	template<typename T>
	static bool IsInvalidValue(const T&) { return false; }

	// --- Specialize for pointer types ---
	template<typename T>
	static bool IsInvalidValue(T* ptr) { return ptr == nullptr; }

	// --- Specialize for bool ---
	static bool IsInvalidValue(const bool& value) { return !value; }

	static inline const char* FileNameFromPath(const char* path)
	{
		const char* file = strrchr(path, '/');
		return file ? file + 1 : path;
	}


	~FLogger() = default;
	
};


inline DEFINE_LOG_CATEGORY(LogGIS_Debug);
inline DEFINE_LOG_CATEGORY(LogGIS_Info);
inline DEFINE_LOG_CATEGORY(LogGIS_Warn);
inline DEFINE_LOG_CATEGORY(LogGIS_Error);
inline DEFINE_LOG_CATEGORY(LogGIS_Fatal);



// Macros for automatic file/line/expression capture

#if !UE_BUILD_SHIPPING
#define GIS_HANDLE(expr, level) \
(FLogger::Get().Handle((expr), level, #expr, __FILE__, __LINE__))
#define GIS_HANDLE_OR(expr, fallback, level) \
(FLogger::Get().Handle((expr),(fallback),level, #expr, __FILE__, __LINE__))
#define GIS_HANDLE(expr, level) \
(FLogger::Get().Handle((expr), level, #expr, __FILE__, __LINE__))
#define GIS_HANDLE_IF(expr) if(GIS_HANDLE(expr, FLoggerLevel::Warn))
#define GIS_HANDLE_MSG(x,level,msg) FLogger::HandleMsg((x), level, FString(msg), __FILE__, __LINE__)

#define GIS_DEBUG(x)  FLogger::Handle((x), FLoggerLevel::Debug, #x, __FILE__, __LINE__)
#define GIS_INFO(x)   FLogger::Handle((x), FLoggerLevel::Info,  #x, __FILE__, __LINE__)
#define GIS_WARN(x)   FLogger::Handle((x), FLoggerLevel::Warn,  #x, __FILE__, __LINE__)
#define GIS_ERROR(x)  FLogger::Handle((x), FLoggerLevel::Error, #x, __FILE__, __LINE__)
#define GIS_FATAL(x)  FLogger::Handle((x), FLoggerLevel::Fatal, #x, __FILE__, __LINE__)
#define GIS_DEBUG_MSG(x, msg)  FLogger::HandleMsg((x), FLoggerLevel::Debug, FString(msg), __FILE__, __LINE__)
#define GIS_INFO_MSG(x, msg)   FLogger::HandleMsg((x), FLoggerLevel::Info,  FString(msg), __FILE__, __LINE__)
#define GIS_WARN_MSG(x, msg)   FLogger::HandleMsg((x), FLoggerLevel::Warn,  FString(msg), __FILE__, __LINE__)
#define GIS_ERROR_MSG(x, msg)  FLogger::HandleMsg((x), FLoggerLevel::Error, FString(msg), __FILE__, __LINE__)
#define GIS_FATAL_MSG(x, msg)  FLogger::HandleMsg((x), FLoggerLevel::Fatal, FString(msg), __FILE__, __LINE__)

#else
#define GIS_DEBUG(x) (x)
#define GIS_INFO(x) (x)
#define GIS_WARN(x) (x)
#define GIS_ERROR(x) (x)
#define GIS_FATAL(x) (x)
#define GIS_HANDLE(expr, level) (expr) // compiled away
#define GIS_HANDLE_OR(expr, fallback, level)(expr)
#define GIS_HANDLE(expr, level) (expr)
#define GIS_HANDLE_IF(expr) (expr)


#define GIS_DEBUG_MSG(x, msg)  (x)
#define GIS_INFO_MSG(x, msg)   (x)
#define GIS_WARN_MSG(x, msg)   (x)
#define GIS_ERROR_MSG(x, msg)  (x)
#define GIS_FATAL_MSG(x, msg)  (x)
#endif
