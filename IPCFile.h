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

#if defined(_WIN64) || defined(_WIN32) // windows
	#if !defined(FORCEINLINE)
		#define FORCEINLINE __forceinline
	#endif
	#if !defined(FASTCALL)
		#define FASTCALL __fastcall
	#endif
#else // linux
	#if !defined(FORCEINLINE)
		#define FORCEINLINE __attribute__((always_inline)) 
	#endif
	#if !defined(FASTCALL)
		#define FASTCALL __attribute__((fastcall)) 
	#endif
#endif

#define NEWLINE_CHAR			'\n'
#define DELIM_CHAR				','
#define WRITE_MODE				"w"
#define READ_MODE				"r"
#define FILE_EXTENSION			".ipcf"
#define ATTRIBUTE_DELIM_CHAR	':'
#define TRUE_STRING				"1"
#define FALSE_STRING			"0"
#define FILE_FOOTER_STRING		"EOF"

#define GET_REQUEST_STRING		"GET"
#define SET_REQUEST_STRING		"SET"
#define FILE_DELIM_CHAR			"#"

#define ATTRIBUTE_CHAR_MAX		1024

#define IPC_PLATFORM_CACHE_LINE_SIZE	64
#define IPC_ALIGN_TO_CACHE_LINE			alignas(IPC_PLATFORM_CACHE_LINE_SIZE)

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

	enum class ERequestType : uint8_t
	{
		GET,
		SET
	};

	struct FToken
	{
		FToken()
			: Token(Incrementor.fetch_add(1, std::memory_order_seq_cst))
		{
		}
		
		FORCEINLINE uint64_t ResetSetToken()
		{
			Token = Incrementor.fetch_add(1, std::memory_order_seq_cst);
			return Token;
		}

	private:
		uint64_t Token;
		inline static std::atomic<uint64_t> Incrementor = {0};
	};
	
	struct FAttributeStringPair
	{
		std::string KeyString;
		std::string ValueString;
	};
	
	template<typename T>
	class IPC_ALIGN_TO_CACHE_LINE IAttribute
	{
		// TODO: add static_asserts on constructibility of template T
		
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
			class IPC_ALIGN_TO_CACHE_LINE IColumnAttribute : public IAttribute<T>
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

			typedef IColumnAttribute<std::string> IColumnAttributeString;
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
	
	class IPC_ALIGN_TO_CACHE_LINE FPlayerAttributeList
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

		EAttributeName& operator[](const int Index)
		{
			return AttributesInUse[Index];
		}
		
		EAttributeName operator[](const int Index) const
		{
			return AttributesInUse[Index];
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

		FORCEINLINE IAttributeString GetPlayerAuthID() const noexcept
		{
			return PlayerAuthID;
		}

		FORCEINLINE IAttributeString GetPlayerName() const noexcept
		{
			return PlayerName;
		}

		FORCEINLINE IAttributeBool GetIsOnline() const noexcept
		{
			return IsOnline;
		}

		FORCEINLINE bool IsEmpty() const noexcept
		{
			return AttributesInUse.size() == 0;
		}

		FORCEINLINE size_t Size() const noexcept
		{
			return AttributesInUse.size();
		}
		
	private:
		FORCEINLINE bool CheckIfAttributeIsAlreadySet(
			const EAttributeName& InAttributeName) const noexcept
		{
			if(AttributesInUse.empty())
			{
				return false;
			}
			
			for(int i = 0; i < AttributesInUse.size(); ++i)
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
	
	class IPCFileManager final
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
		
		IPCFileManager() = default;
		~IPCFileManager() = default;

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


		/* TODO: Create functions for getting data,
		 * they will need a new type used to simply list
		 * what attributes are desired..
		 *
		 * Also.. both the getter & setter functions will also need some
		 * sort of id token, one for the transaction itself, the other
		 * will be used to id the player invoking the request
		 */
		
		static FORCEINLINE bool WriteAttributeVectorToFile(
			const std::string& FileLocation,
			const std::vector<FPlayerAttributeList>& InAttributeArray)
		{
			if(InAttributeArray.size() == 0)
			{
				return false;
			}
			
			std::string CompleteFileString = "";
			for(int i = 0; i < InAttributeArray.size(); ++i)
			{
				std::string CurrentLine = "";
				const FPlayerAttributeList PlayerAttributes =
					InAttributeArray[i];

				// Combine each attribute key and value into a string
				// then append it to the line.
				FAttributeStringPair StringPair;
				std::string AttributeString = "";
				for(int j = 0; j < PlayerAttributes.Size(); ++j)
				{
					if(PlayerAttributes[j] == TableKey_PlayerAuthID.Name)
					{
						StringPair.KeyString = TableKey_PlayerAuthID.Key;
						StringPair.ValueString =
							PlayerAttributes.GetPlayerAuthID().Value;
					}
					else if(PlayerAttributes[j] == TableKey_PlayerName.Name)
					{
						StringPair.KeyString = TableKey_PlayerName.Key;
						StringPair.ValueString =
							PlayerAttributes.GetPlayerName().Value;
					}
					else if(PlayerAttributes[j] == TableKey_IsOnline.Name)
					{
						StringPair.KeyString = TableKey_IsOnline.Key;
						StringPair.ValueString =
							(PlayerAttributes.GetIsOnline().Value) ?
								(TRUE_STRING) : (FALSE_STRING);
					}

					AttributeString = StringPair.KeyString +
						ATTRIBUTE_DELIM_CHAR + StringPair.ValueString + DELIM_CHAR;
					CurrentLine.append(AttributeString);
				}

				CurrentLine += NEWLINE_CHAR; // add the newline char onto the end
				CompleteFileString.append(CurrentLine); // add the line to the file text
			}
			CompleteFileString.append(FILE_FOOTER_STRING); // add the footer

			// Generate a unique name for this set request file
			std::string UniqueFileName;
			GeneratorUniqueFileName(UniqueFileName, ERequestType::SET);
			const std::string FullPath = FileLocation + UniqueFileName;
			
			// write the data to the file
			FILE *File;
			fopen_s(&File, FullPath.c_str(), WRITE_MODE);
			if(!File)
			{
				return false;
			}
			fputs(CompleteFileString.c_str(), File);
			fclose(File);
			return true;
		}

		static FORCEINLINE void ReadFromFileAndGetAttributes(
			const std::string& FileLocation,
			std::vector<FPlayerAttributeList>& OutAttributeVector)
		{
			std::vector<std::string> FileLines;
			ReadLinesFromFile(FileLocation, FileLines);
			// Split attributes into strings
			for(int i = 0; i < FileLines.size(); ++i)
			{
				// Split the line into attributes
				const std::string LineString = FileLines[i];
				std::vector<std::string> AttributeStrings;
				SplitLineIntoAttributeStrings(LineString, AttributeStrings);
				// Split each attribute into is key/value pair as stringss
				std::vector<FAttributeStringPair> SplitAttributes;
				SplitAttributeStrings(AttributeStrings, SplitAttributes);
				// Put attributes into FPlayerAttributeList
				FPlayerAttributeList PlayerAttributes;
				ConvertSplitAttributesToPlayerAttributes(
					SplitAttributes, PlayerAttributes);
				// Make sure there was actually something to update
				if(!PlayerAttributes.IsEmpty())
				{
					// Push the now assembled FPlayerAttributeList into the vector
					OutAttributeVector.push_back(PlayerAttributes);
				}
			}
		}

	private:
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
			StreamBuffer << File.rdbuf(); // hate doing this
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

		static FORCEINLINE void SplitLineIntoAttributeStrings(
			const std::string& LineString,
			std::vector<std::string>& AttributeStrings
			)
		{
			std::string StringBuffer = "";
			for(int i = 0; i < LineString.size(); ++i)
			{
				if(LineString[i] == DELIM_CHAR)
				{
					AttributeStrings.push_back(StringBuffer);
					StringBuffer.erase();
				}
				else
				{
					StringBuffer += LineString[i];
				}
			}
		}

		static FORCEINLINE void SplitAttributeStrings(
			const std::vector<std::string>& AttributeStrings,
			std::vector<FAttributeStringPair>& SplitAttributes)
		{
			for(int i = 0; i < AttributeStrings.size(); ++i)
			{
				FAttributeStringPair StringPairBuffer;
				const std::string AttributeString = AttributeStrings[i];
				const size_t StringSize = AttributeString.size();
				for(int j = 0; j < StringSize; ++j)
				{
					if(AttributeString[j] == ATTRIBUTE_DELIM_CHAR)
					{
						for(int k = 0; k < j; ++k)
						{
							StringPairBuffer.KeyString += AttributeString[k];
						}
							
						// add the rest of the string after the delim char
						// to the value string
						for(int k = (j + 1); k < StringSize; ++k)
						{
							StringPairBuffer.ValueString += AttributeString[k];
						}
					}
				}
				SplitAttributes.push_back(StringPairBuffer);
			}
		}

		static FORCEINLINE void ConvertSplitAttributesToPlayerAttributes(
			const std::vector<FAttributeStringPair>& SplitAttributes,
			FPlayerAttributeList& PlayerAttributes)
		{
			for(int i = 0; i < SplitAttributes.size(); ++i)
			{
				const FAttributeStringPair KeyValueStringPair =
					SplitAttributes[i];

				// Check if the key and value are the same...
				if(KeyValueStringPair.KeyString ==
					KeyValueStringPair.ValueString)
				{
					// TODO handle this
				}

				// Check to see which attribute it is, then add it the list
				// NOTE: Can't use a switch here because we're just checking
				// equality on strings
				if(KeyValueStringPair.KeyString == TableKey_PlayerAuthID.Key)
				{
					const IAttributeString Buffer(
						EAttributeTypes::STRING,
						EAttributeName::PLAYER_AUTH,
						KeyValueStringPair.ValueString);
					PlayerAttributes.SetPlayerAuthID(Buffer);
				}
				else if(KeyValueStringPair.KeyString == TableKey_PlayerName.Key)
				{
					const IAttributeString Buffer(
						EAttributeTypes::STRING,
						EAttributeName::PLAYER_NAME,
						KeyValueStringPair.ValueString);
					PlayerAttributes.SetPlayerName(Buffer);
				}
				else if(KeyValueStringPair.KeyString == TableKey_IsOnline.Key)
				{
					const IAttributeBool Buffer(
						EAttributeTypes::BOOL,
						EAttributeName::IS_ONLINE,
						(KeyValueStringPair.ValueString == TRUE_STRING));
					PlayerAttributes.SetIsOnline(Buffer);
				}
			}
		}
		
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

		static FORCEINLINE void GeneratorUniqueFileName(std::string& Out,
			const ERequestType& RequestType)
		{
			const uint64_t UniqueID = FileNameIncrementor.fetch_add(1,
				std::memory_order_seq_cst);
			const std::string UniqueIDString = std::to_string(UniqueID);
			const std::string RequestTypeString = (RequestType == ERequestType::GET) ?
				(GET_REQUEST_STRING) : (SET_REQUEST_STRING);
			// Output the file name
			Out = RequestTypeString + FILE_DELIM_CHAR + UniqueIDString;
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

	private:
		inline static std::atomic<uint64_t> FileNameIncrementor; 
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

#undef GET_REQUEST_STRING		
#undef SET_REQUEST_STRING		
#undef FILE_DELIM_CHAR			

#undef ATTRIBUTE_CHAR_MAX

#undef IPC_PLATFORM_CACHE_LINE_SIZE
#undef IPC_ALIGN_TO_CACHE_LINE

#endif
