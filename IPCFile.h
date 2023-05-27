#ifndef IPC_FILE_H
#define IPC_FILE_H

// C
#include <cstdio>
#include <cstdlib>
// C++
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

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

#define NEWLINE_CHAR			'\n'
#define DELIM_CHAR				','
#define WRITE_MODE				"w"
#define READ_MODE				"r"
#define FILE_EXTENSION			".txt"
#define ATTRIBUTE_DELIM_CHAR	':'
#define TRUE_STRING				"1"
#define FALSE_STRING			"0"

#define ATTRIBUTE_CHAR_MAX		1024

#define IPCFILE_DEBUG_ENABLED 0
#if IPCFILE_DEBUG_ENABLED
	#define IPCASSERT(_CONDITION_, _ERR_STRING_) if((_CONDITION_)) { \
		printf("\n!! "); \
		printf((_ERR_STRING_)); \
		printf(" !!\n"); \
		system("pause"); \
		exit(1); \
	}
#else
	#define IPCASSERT(_CONDITION_, _ERR_STRING_)
#endif

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

	struct FAttributeStringPair
	{
		std::string KeyString;
		std::string ValueString;
	};
	
	template<typename T>
	class IAttribute
	{
	public:
		EAttributeTypes Type;
		EAttributeName Name;
		T Value;

		IAttribute()
			: Type(EAttributeTypes::NONE),
			Name(EAttributeName::NONE),
			Value(T())
		{
		}
		
		IAttribute(const EAttributeTypes& InType,
			const EAttributeName& InName,
			const T& InValue)
			: Type(InType),
			Name(InName),
			Value(InValue)
		{
		}
	};

	typedef IAttribute<std::string> IAttributeString;
	typedef IAttribute<int>			IAttributeInt;
	typedef IAttribute<float>		IAttributeFloat;
	typedef IAttribute<bool>		IAttributeBool;
	
	namespace TableDataStatics
	{
		namespace Internal
		{
			template<typename T>
			class IColumnAttribute : public IAttribute<T>
			{
			public:
				T Key = IAttribute<T>::Value;
				
				IColumnAttribute()
					: IAttribute<T>()
				{
				}

				IColumnAttribute(const EAttributeTypes& InType,
									const EAttributeName& InName,
									const T& InKey)
					: IAttribute<T>(InType, InName, InKey)
				{
				}
			};

			typedef IColumnAttribute<std::string>	IColumnAttributeString;
		}
		
		static constexpr int NumberOfAttributes = 3;
		
		static const Internal::IColumnAttributeString TableKey_PlayerAuthID =
			Internal::IColumnAttributeString(
				EAttributeTypes::STRING,
				EAttributeName::PLAYER_AUTH,
				std::string("PlayerID"));
		
		static const Internal::IColumnAttributeString TableKey_PlayerName =
			Internal::IColumnAttributeString(
				EAttributeTypes::STRING,
				EAttributeName::PLAYER_NAME,
				std::string("PlayerName"));
		
		static const Internal::IColumnAttributeString TableKey_IsOnline =
			Internal::IColumnAttributeString(
				EAttributeTypes::BOOL,
				EAttributeName::IS_ONLINE,
				std::string("IsOnline"));
	}
	
	class FPlayerAttributeList
	{
	public:
		static constexpr int TotalNumberOfAttributes =
			TableDataStatics::NumberOfAttributes;
		
		FPlayerAttributeList()
			: AttributesInUse{},
			PlayerAuthID(),
			PlayerName(),
			IsOnline()
		{
		}

		FORCEINLINE void SetPlayerAuthID(const IAttributeString& InPlayerAuthID)
		{
			if(!CheckIfAttributeIsAlreadySet(InPlayerAuthID.Name))
			{
				AttributesInUse.push_back(InPlayerAuthID.Name);
			}
			PlayerAuthID = InPlayerAuthID;
		}

		FORCEINLINE void SetPlayerName(const IAttributeString& InPlayerName)
		{
			if(!CheckIfAttributeIsAlreadySet(InPlayerName.Name))
			{
				AttributesInUse.push_back(InPlayerName.Name);
			}
			PlayerName = InPlayerName;
		}

		FORCEINLINE void SetIsOnline(const IAttributeBool& InIsOnline)
		{
			if(!CheckIfAttributeIsAlreadySet(InIsOnline.Name))
			{
				AttributesInUse.push_back(InIsOnline.Name);
			}
			IsOnline = InIsOnline;
		}

		FORCEINLINE IAttributeString GetPlayerAuthID() const
		{
			return PlayerAuthID;
		}

		FORCEINLINE IAttributeString GetPlayerName() const
		{
			return PlayerName;
		}

		FORCEINLINE IAttributeBool GetIsOnline() const
		{
			return IsOnline;
		}

		FORCEINLINE bool IsEmpty()
		{
			return AttributesInUse.size() == 0;
		}
		
	private:
		FORCEINLINE bool CheckIfAttributeIsAlreadySet(
			const EAttributeName& InAttributeName)
		{
			if(AttributesInUse.empty())
			{
				return false;
			}
			
			for(int i = 0; AttributesInUse.size(); ++i)
			{
				if(AttributesInUse[i] == InAttributeName)
				{
					return true;
				}
			}
			
			return false;
		}
		
		std::vector<EAttributeName> AttributesInUse;
		
		IAttributeString	PlayerAuthID;
		IAttributeString	PlayerName;
		IAttributeBool		IsOnline;
	};
	
	class IPCFileManager
	{
	public:
		template<typename T> using FColumnAttribute	=
			TableDataStatics::Internal::IColumnAttribute<T>;
		
		inline static const FColumnAttribute<std::string> TableKey_PlayerAuthID =
			TableDataStatics::TableKey_PlayerAuthID;
		inline static const FColumnAttribute<std::string> TableKey_PlayerName =
			TableDataStatics::TableKey_PlayerName;
		inline static const FColumnAttribute<std::string> TableKey_IsOnline =
			TableDataStatics::TableKey_IsOnline;
		
		IPCFileManager();
		~IPCFileManager();

		/**
		 * \brief Create a file with a given name and location.
		 * \param FileStream
		 * \param FileName Name of the file.
		 * \param Directory Full directory path to put the file in.
		 */
		static FORCEINLINE void CreateFile(
			FILE *FileStream,
			const std::string& FileName,
			const std::string& Directory)
		{
			const std::string FileLocation = Directory + "\\" + FileName;
			fopen_s(&FileStream, FileLocation.c_str(), WRITE_MODE);
		}

		/**
		 * \brief Delete a certain file.
		 * \param FileName Name of the file.
		 * \param Directory Full directory path to put the file in.
		 */
		static FORCEINLINE void DeleteFile(
			const std::string& FileName,
			const std::string& Directory)
		{
			const std::string FileLocation = Directory + "\\" + FileName;
			remove(FileLocation.c_str());
		}

		/**
		 * \brief Write a list of player attribute strings to a certain file.
		 * \param FileName Name of the file.
		 * \param Directory Full directory path to put the file in.
		 * \param StringArray Vector of player attribute strings to write.
		 */
		static FORCEINLINE bool WriteToFile(
			const std::string& FileName,
			const std::string& Directory,
			std::vector<std::string>& StringArray)
		{
			std::string StringBuilder;
			for(int i = 0; i < StringArray.size(); ++i)
			{
				StringBuilder.append(StringArray.back());
				StringArray.pop_back();
			}
			
			const std::string FileLocation =
				Directory + "\\" + FileName + "-" + GetSystemTimeAsString() + FILE_EXTENSION;
			FILE *File;
			fopen_s(&File, FileLocation.c_str(), WRITE_MODE);
			if(!File)
			{
				return false;
			}

			fputs(StringBuilder.c_str(), File);
			fclose(File);
			return true;
		}

		/**
		 * \brief Read a list of player attribute strings from a file, stores
		 * them in an output variable.
		 * \param FileLocation the location of the file (C:\\dir\\dir\\file)
		 * \param OutStringArray Output string vector to store the player attribute
		 * strings in.
		 */
		static FORCEINLINE bool ReadLinesFromFile(
			const std::string& FileLocation,
			std::vector<std::string>& OutStringArray)
		{
			const std::ifstream File(FileLocation);
			std::stringstream StreamBuffer;
			StreamBuffer.set_rdbuf(File.rdbuf());
			const std::string FileText = StreamBuffer.str();
			if(FileText.size() == 0)
			{
				return false;
			}

			std::string LineBuffer = "";
			for(int i = 0; i < FileText.size(); ++i)
			{
				if(FileText[i] == NEWLINE_CHAR)
				{
					OutStringArray.push_back(LineBuffer);
					LineBuffer.erase();
				}
				else
				{
					LineBuffer += FileText[i];
				}
			}
			
			return (OutStringArray.size() > 0);
		}

		static FORCEINLINE void ReadFromFileAndSplitAttributes(
			const std::string& FileLocation,
			std::vector<FPlayerAttributeList>& OutAttributeArray)
		{
			std::vector<std::string> FileLines;
			ReadLinesFromFile(FileLocation, FileLines);
			// Split attributes into strings
			for(int i = 0; i < FileLines.size(); ++i)
			{
				const std::string Line = FileLines.back();
				std::vector<std::string> AttributeStrings;
				std::string StringBuffer = "";
				for(int j = 0; j < Line.size(); ++j)
				{
					if(Line[j] == DELIM_CHAR)
					{
						AttributeStrings.push_back(StringBuffer);
						StringBuffer.erase();
					}
					else
					{
						StringBuffer += Line[j];
					}
				}
				// split each attribute into is key/value pair as strings
				std::vector<FAttributeStringPair> SplitAttributes;
				for(int j = 0; j < AttributeStrings.size(); ++j)
				{
					FAttributeStringPair StringPairBuffer;
					const std::string AttributeString = AttributeStrings[j];
					const int StringSize = AttributeString.size();
					for(int k = 0; k < StringSize; ++k)
					{
						if(AttributeString[k] == ATTRIBUTE_DELIM_CHAR)
						{
							// add the rest of the string after the delim char
							// to the value string
							for(int x = (k + 1); x < StringSize; ++x)
							{
								StringPairBuffer.ValueString += AttributeString[x];
							}
							continue;
						}
						// if we got here we haven't hit the delim yet, so
						// add this char to the key string
						StringPairBuffer.KeyString += AttributeString[k];
					}
					SplitAttributes.push_back(StringPairBuffer);
				}
				// Put attributes into FPlayerAttributeList
				FPlayerAttributeList PlayerAttributes;
				for(int j = 0; j < SplitAttributes.size(); ++j)
				{
					const FAttributeStringPair KeyValueStringPair = SplitAttributes[j];

					// Check if the key and value are the same...
					if(KeyValueStringPair.KeyString == KeyValueStringPair.ValueString)
					{
						// TODO handle this
					}

					switch(KeyValueStringPair.KeyString)
					{
						case TableKey_PlayerAuthID.Key: // Player Auth
							{
								const IAttributeString Buffer(
									EAttributeTypes::STRING,
									EAttributeName::PLAYER_AUTH,
									KeyValueStringPair.ValueString);
								PlayerAttributes.SetPlayerAuthID(Buffer);
							}
							break;
						case TableKey_PlayerName.Key: // Player Name
							{
								const IAttributeString Buffer(
									EAttributeTypes::STRING,
									EAttributeName::PLAYER_NAME,
									KeyValueStringPair.ValueString);
								PlayerAttributes.SetPlayerName(Buffer);
							}
							break;
						case TableKey_IsOnline.Key: // Is Online
							{
								const IAttributeBool Buffer(
									EAttributeTypes::BOOL,
									EAttributeName::IS_ONLINE,
									(KeyValueStringPair.ValueString == TRUE_STRING));
								PlayerAttributes.SetIsOnline(Buffer);
							}
							break;
						
						default:
							break;
					}
				}
				if(!PlayerAttributes.IsEmpty())
				{
					OutAttributeArray.push_back(PlayerAttributes);
				}
				FileLines.pop_back();
			}
		}
		
	protected:
		/**
		 * \brief Convert the value of an attribute to a string.
		 * \tparam T Template type for the attribute type.
		 * \param Attribute Attribute to stringify.
		 * \param Out String to store the output in.
		 */
		template<typename T>
		static FORCEINLINE void FASTCALL StringifyAttribute(
			const IAttribute<T>& Attribute,
			std::string& Out)
		{
			// TODO
			switch(Attribute.Type)
			{
				case EAttributeTypes::INT:
					break;
				case EAttributeTypes::FLOAT:
					break;
				case EAttributeTypes::STRING:
					break;
				case EAttributeTypes::BOOL:
					break;
				default: ;
			}
		}

		static FORCEINLINE bool FASTCALL GetListOfFiles(
			const std::string& Directory,
			std::vector<std::string>& OutFileList)
		{
			if(!std::filesystem::exists(Directory))
			{
				return false;
			}
			
			for(const auto& File :
				std::filesystem::directory_iterator(Directory))
			{
				OutFileList.push_back(File.path().string());
			}
			return OutFileList.size() > 0;
		}
		
		static FORCEINLINE std::string FASTCALL GetSystemTimeAsString() noexcept
		{
			// TODO
			return "";
		}

		static FORCEINLINE void FASTCALL GetSystemTime() noexcept
		{
			// TODO
		}
	};
}

#undef NEWLINE_CHAR
#undef DELIM_CHAR
#undef WRITE_MODE
#undef READ_MODE
#undef FILE_EXTENSION
#undef ATTRIBUTE_DELIM_CHAR
#undef TRUE_STRING
#undef FALSE_STRING

#undef ATTRIBUTE_CHAR_MAX

#undef IPCFILE_DEBUG_ENABLED
#undef IPCASSERT

#endif
