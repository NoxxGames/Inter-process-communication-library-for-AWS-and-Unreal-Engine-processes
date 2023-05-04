#ifndef IPC_FILE_H
#define IPC_FILE_H

#include "stdio.h"
#include "stdlib.h"

#include <assert.h>
#include <fstream>
#include <string>
#include <vector>

#if defined(_WIN64) || defined(_WIN32)
	#if !defined(FORCEINLINE)
		#define FORCEINLINE __forceinline
	#endif
	#if !defined(FASTCALL)
		#define FASTCALL __fastcall
	#endif
#else
	#if !defined(FORCEINLINE)
		#define FORCEINLINE __attribute__ ((always_inline)) 
	#endif
	#if !defined(FASTCALL)
		#define FASTCALL __attribute__ ((fastcall)) 
	#endif
#endif

#define NEWLINE_CHAR	'\n'
#define DELIM_CHAR		','

#define IPCASSERT(_CONDITION_, _ERR_STRING_) if((_CONDITION_)) { \
	printf("\n!! "); \
	printf((_ERR_STRING_)); \
	printf(" !!\n"); \
	system("pause"); \
	exit(1); \
}

namespace IPCFile
{
	enum class EAttributeTypes : uint8_t
	{
		NONE,
		STRING,
		INT,
		FLOAT,
		BOOL
	};

	enum class EAttributeName : uint8_t
	{
		NONE,
		PLAYER_AUTH,
		PLAYER_NAME,
		IS_ONLINE
	};

	template<typename T>
	struct FAttribute
	{
		EAttributeTypes Type;
		EAttributeName Name;
		T Value;

		FAttribute()
			: Type(EAttributeTypes::NONE),
			Name(EAttributeName::NONE),
			Value(T())
		{
		}
		
		FAttribute(const EAttributeTypes& InType,
					const EAttributeName& InName,
					const T& InValue)
			: Type(InType),
			Name(InName),
			Value(InValue)
		{
		}
	};

	typedef FAttribute<std::string> FAttributeString;
	typedef FAttribute<int>			FAttributeInt;
	typedef FAttribute<float>		FAttributeFloat;
	typedef FAttribute<bool>		FAttributeBool;
	
	namespace TableDataStatics
	{
		static constexpr int NumberOfAttributes = 3;
		
		static const FAttributeString TableKey_PlayerAuthID =
			FAttributeString(
				EAttributeTypes::STRING,
				EAttributeName::PLAYER_AUTH,
				std::string("PlayerID"));
		
		static const FAttributeString TableKey_PlayerName =
			FAttributeString(
				EAttributeTypes::STRING,
				EAttributeName::PLAYER_NAME,
				std::string("PlayerName"));
		
		static const FAttributeBool TableKey_IsOnline =
			FAttributeBool(
				EAttributeTypes::BOOL,
				EAttributeName::IS_ONLINE,
				false);
	}

	template<uint8_t TNumberOfAttributes = 0>
	struct FPlayerAttributeList
	{
		static_assert(TNumberOfAttributes > 0, "NumberOfAttributes cannot be zero!");
		
		static constexpr int NumberOfAttributes = TableDataStatics::NumberOfAttributes;

		EAttributeName Attributes[TNumberOfAttributes];
		
		FAttributeString	PlayerAuthID;
		FAttributeString	PlayerName;
		FAttributeBool		IsOnline;

		FPlayerAttributeList()
			: Attributes{},
			PlayerAuthID(),
			PlayerName(),
			IsOnline()
		{
		}
	};
	
	class IPCFileManager
	{
	public:
		IPCFileManager();
		~IPCFileManager();

		/**
		 * \brief Create a file with a given name and location.
		 * \param FileName Name of the file.
		 * \param Directory Full directory path to put the file in.
		 */
		static FORCEINLINE void CreateFile(const std::string& FileName,
										   const std::string& Directory)
		{
			// TODO
		}

		/**
		 * \brief Delete a certain file.
		 * \param FileName Name of the file.
		 * \param Directory Full directory path to put the file in.
		 */
		static FORCEINLINE void DeleteFile(const std::string& FileName,
										   const std::string& Directory)
		{
			// TODO
		}

		/**
		 * \brief Write a list of player attribute strings to a certain file.
		 * \param FileName Name of the file.
		 * \param Directory Full directory path to put the file in.
		 * \param StringArray Vector of player attribute strings to write.
		 */
		static FORCEINLINE void WriteToFile(const std::string& FileName,
											const std::string& Directory,
											const std::vector<std::string>& StringArray)
		{
			// TODO
		}

		/**
		 * \brief Read a list of player attribute strings from a file, stores
		 * them in an output variable.
		 * \param FileName Name of the file.
		 * \param Directory Full directory path to put the file in.
		 * \param OutStringArray Output string vector to store the player attribute
		 * strings in.
		 */
		static FORCEINLINE void ReadFromFile(const std::string& FileName,
											 const std::string& Directory,
											 std::vector<std::string>& OutStringArray)
		{
			// TODO

			std::string FileText = "";
			GetFileData(FileName, Directory, FileText);
		}
		
	protected:
		/**
		 * \brief Convert the value of an attribute to a string.
		 * \tparam T Template type for the attribute type.
		 * \param Attribute Attribute to stringify.
		 * \param Out String to store the output in.
		 */
		template<typename T>
		static FORCEINLINE void FASTCALL StringifyAttribute(const FAttribute<T>& Attribute,
													std::string& Out)
		{
			// TODO switch on EAttributeTypes variable in FAttribute
			// to check which type we need to convert
		}
		
		static FORCEINLINE void FASTCALL GetSystemTimeAsString(std::string& Out)
		{
			// TODO
		}

		static FORCEINLINE void FASTCALL GetSystemTime()
		{
			// TODO
		}
		
	private:
		static FORCEINLINE void FASTCALL GetFileData(const std::string& FileName,
													 const std::string& Directory,
													 std::string& Out)
		{
			// TODO
		}

		template<uint8_t TNumberOfAttributes = 0>
		static FORCEINLINE FPlayerAttributeList<TNumberOfAttributes>
			FASTCALL CreateAttributeList(const std::string& AttributeStrings)
		{
			// TODO
			IPCASSERT(TNumberOfAttributes == 0, "NumberOfAttributes cannot be zero! at CreateAttributeList()");
			return FPlayerAttributeList<TNumberOfAttributes>();
		}
	};
	
}

#undef TEXT
#undef NEWLINE_CHAR
#undef DELIM_CHAR

#endif
