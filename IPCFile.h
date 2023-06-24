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
#include <thread>

#if defined(_WIN64) || defined(_WIN32) // windows
	#if !defined(FORCEINLINE)
		#define FORCEINLINE __forceinline
	#endif
#else // linux
	#if !defined(FORCEINLINE)
		#define FORCEINLINE __attribute__((always_inline)) 
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
#define GET_RESPONSE_REQUEST_STRING		"GETRESPONSE"
#define SET_REQUEST_STRING		"SET"
#define FILE_DELIM_CHAR			'#'
#define FILE_TIME_DELIM_CHAR	'-'

#define ATTRIBUTE_CHAR_MAX		1024

#define UE_BUFFER_MAX			524288
#define AWS_BUFFER_MAX			524288

#define UE_BUFFER_TICK_RATE		128
#define AWS_BUFFER_TICK_RATE	128

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
		GET_RESPONSE,
		SET
	};

	struct FToken
	{
		FToken()
			: Token(Incrementor.fetch_add(1, std::memory_order_seq_cst))
		{
		}
		
		FORCEINLINE uint64_t GenerateNewToken()
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
			class IPC_ALIGN_TO_CACHE_LINE IColumnAttribute final : public IAttribute<T>
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
	
	class IPC_ALIGN_TO_CACHE_LINE FPlayerAttributeList final
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

	private:
		std::vector<EAttributeName> AttributesInUse;
		
		IAttributeString	PlayerAuthID;
		IAttributeString	PlayerName;
		IAttributeBool		IsOnline;
	};

	class IPC_ALIGN_TO_CACHE_LINE FIPCRequest
	{
	public:
		static constexpr int TotalNumberOfAttributes =
			TableDataStatics::NumberOfAttributes;

		FIPCRequest(const IAttributeString& InPlayerAuthID)
			: PlayerAuthID(InPlayerAuthID)
		{
		}

		virtual ~FIPCRequest() = default;
		
		virtual FORCEINLINE IAttributeString GetPlayerAuthID() const noexcept
		{
			return PlayerAuthID;
		}
		
		virtual FORCEINLINE std::string GetPlayerAuthIDString() const noexcept
		{
			return PlayerAuthID.Value;
		}

		virtual FORCEINLINE bool IsEmpty() const = delete;
		virtual FORCEINLINE size_t Size() const = delete;
		
	protected:
		const IAttributeString PlayerAuthID;
	};

	/*
	 * GetRequest
	 */
	class IPC_ALIGN_TO_CACHE_LINE FGetRequest final : public FIPCRequest
	{
	public:
		FGetRequest(const IAttributeString& InPlayerAuthID)
			: FIPCRequest(InPlayerAuthID),
			AttributesToGet{}
		{
		}

		FGetRequest(
			const IAttributeString& InPlayerAuthID,
			std::vector<EAttributeName>& InAttributesToGet)
				: FIPCRequest(InPlayerAuthID),
				AttributesToGet(std::move(InAttributesToGet))
		{
		}

		EAttributeName& operator[](const int Index)
		{
			return AttributesToGet[Index];
		}
		
		EAttributeName operator[](const int Index) const
		{
			return AttributesToGet[Index];
		}

		FORCEINLINE bool AddAttributeToGet(const EAttributeName& InAttributeName)
		{
			if(CheckIfAttributeIsAlreadySet(InAttributeName))
			{
				return false;
			}

			AttributesToGet.push_back(InAttributeName);
			return true;
		}

		virtual FORCEINLINE bool IsEmpty() const noexcept override
		{
			return Size() == 0;
		}

		virtual FORCEINLINE size_t Size() const noexcept override
		{
			return AttributesToGet.size();
		}
		
	private:
		FORCEINLINE bool CheckIfAttributeIsAlreadySet(
			const EAttributeName& InAttributeName) const noexcept
		{
			if(AttributesToGet.empty() ||
				InAttributeName == TableDataStatics::TableKey_PlayerAuthID.Name)
			{
				return false;
			}
			
			for(int i = 0; i < AttributesToGet.size(); ++i)
			{
				if(AttributesToGet[i] == InAttributeName)
				{
					return true;
				}
			}
			
			return false;
		}
		
	private:
		std::vector<EAttributeName> AttributesToGet;
	};

	/*
	 * SetRequest
	 */
	class IPC_ALIGN_TO_CACHE_LINE FSetRequest final : public FIPCRequest
	{
	public:
		FSetRequest() = delete;
		FSetRequest(
			const IAttributeString& InPlayerAuthID,
			const FPlayerAttributeList& InPlayerAttributes)
				: FIPCRequest(InPlayerAuthID),
				PlayerAttributes(InPlayerAttributes)
		{
		}

		FORCEINLINE const FPlayerAttributeList* GetPlayerAttributeList() const
		{
			return &PlayerAttributes;
		}
		
		virtual FORCEINLINE bool IsEmpty() const noexcept override
		{
			return Size() == 0;
		}

		virtual FORCEINLINE size_t Size() const noexcept override
		{
			return PlayerAttributes.Size();
		}
		
	private:
		const FPlayerAttributeList PlayerAttributes;
	};

	/*
	 * IPCFileManager main static class
	 */
	class IPC_ALIGN_TO_CACHE_LINE IPCFileManager final
	{
		static constexpr int UE_BufferTickRateMS = 1000 / UE_BUFFER_TICK_RATE;
		static constexpr int AWS_BufferTickRateMS = 1000 / AWS_BUFFER_TICK_RATE;

		template<typename T>
		class IPC_ALIGN_TO_CACHE_LINE FRequestBuffer
		{
		public:
			FRequestBuffer() = default;

			// TODO
		protected:
			std::vector<T> RequestBuffer;
		};

		class IPC_ALIGN_TO_CACHE_LINE FGetRequestBuffer
			: public FRequestBuffer<FGetRequest>
		{
		public:
			FGetRequestBuffer()
				: FRequestBuffer()
			{
			}

			// TODO
		};

		class IPC_ALIGN_TO_CACHE_LINE FSetRequestBuffer
			: public FRequestBuffer<FSetRequest>
		{
		public:
			FSetRequestBuffer()
				: FRequestBuffer()
			{
			}

			// TODO
		};
		
		template<int TTickRate = UE_BufferTickRateMS>
		class IPC_ALIGN_TO_CACHE_LINE FBufferThread
		{
		public:
			FBufferThread()
				: IsRunning{false},
				ShouldStop{false}
			{
			}
			
			FORCEINLINE void StartThread()
			{
				if(IsRunning.load())
				{
					return;
				}

				std::thread([=] ()
				{
					IsRunning.store(true, std::memory_order_release);
					for(;;)
					{
						if(ShouldStop.load(std::memory_order_acquire))
						{
							break;
						}

						// TODO process buffer elements, will need a way
						// of checking which buffer this thread should work with.....
						// TODO finish making the set/get request buffer types
						// then declare static variables for the buffers using
						// those types....
						std::this_thread::sleep_for(std::chrono::milliseconds(TTickRate));
					}

					IsRunning.store(false, std::memory_order_release);
				}).detach();
			}

			FORCEINLINE void StopThread()
			{
				ShouldStop.store(true, std::memory_order_release);
			}

			FORCEINLINE bool GetIsRunning() const
			{
				return IsRunning.load(std::memory_order_acquire);
			}

		private:
			std::atomic<bool> IsRunning;
			std::atomic<bool> ShouldStop;
		};
		
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

		/*
		 * Initialize the system on the Unreal Engine side
		 */
		static FORCEINLINE void UE_Initialize()
		{
			// Allocate reserve sizes for unreal buffers
			UE_GetRequestBuffer.reserve(UE_BUFFER_MAX);
			UE_SetRequestBuffer.reserve(UE_BUFFER_MAX);

			// todo create the thread(s) that will manage the buffers
			std::thread([=]()
			{
				
			}).detach();
		}

		/*
		 * Initialize the system on the AWS side
		 */
		static FORCEINLINE void AWS_Initialize()
		{
			// Allocate reserve sizes for aws buffer
			AWS_SetRequestBuffer.reserve(AWS_BUFFER_MAX);

			// todo create the thread(s) that will manage the buffers
		}
		
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
		 *
		 * TODO The current setup is only really good for doing set requests,
		 * as stated above, a new type will be need to simply list what attributes
		 * to get when doing a get request
		 *
		 * TODO The AWS Process will never need to make a get request to the
		 * unreal process, the AWS process will only make a set request to
		 * the unreal process, after the unreal process makes a get request
		 *
		 * UE get request -> AWS Process -> Loads from DynamoDB ->
		 * AWS set request -> UE Process
		 *
		 * UE set request -> AWS Process -> Set in DynamoDB
		 */

		static FORCEINLINE bool UE_AddGetRequestToBuffer(
			const FGetRequest& GetRequest)
		{
			if(GetRequest.IsEmpty())
			{
				return false;
			}
			
			std::string AttributeLine = TableKey_PlayerAuthID.Key +
				ATTRIBUTE_DELIM_CHAR + GetRequest.GetPlayerAuthIDString() +
					DELIM_CHAR;
			for(int i = 0; i < GetRequest.Size(); ++i)
			{
				if(GetRequest[i] == TableKey_PlayerName.Name)
				{
					AttributeLine += TableKey_PlayerName.Key;
				}
				else if(GetRequest[i] == TableKey_IsOnline.Name)
				{
					AttributeLine += TableKey_IsOnline.Key;
				}
				AttributeLine += DELIM_CHAR;
			}
			// Add the new line to the buffer
			UE_GetRequestBuffer.push_back(AttributeLine);
			return true;
		} 
		
		static FORCEINLINE bool UE_AddSetRequestToBuffer(
			const FSetRequest& SetRequest)
		{
			return false;
		}

		static FORCEINLINE bool AWS_AddSetRequestToBuffer(
			const FSetRequest& SetRequest)
		{
			return false;
		}
		
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
		static FORCEINLINE void StringifyAttribute(
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

		static FORCEINLINE bool GetListOfFiles(
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
			std::string RequestTypeString;
			switch(RequestType)
			{
				case ERequestType::GET:
					RequestTypeString = GET_REQUEST_STRING;
					break;
				case ERequestType::GET_RESPONSE:
					RequestTypeString = GET_RESPONSE_REQUEST_STRING;
					break;
				case ERequestType::SET:
					RequestTypeString = SET_REQUEST_STRING;
					break;
				default:
					break;
			}
			// Output the file name
			Out = RequestTypeString + FILE_DELIM_CHAR + UniqueIDString +
				FILE_DELIM_CHAR + GetSystemTimeAsString();
		}
		
		static FORCEINLINE std::string GetSystemTimeAsString() noexcept
		{
			const time_t CurrentTime = time(0);
			const tm *TimeStruct = gmtime(&CurrentTime);
			return std::string(
				std::to_string(TimeStruct->tm_hour) + FILE_TIME_DELIM_CHAR + 
				std::to_string(TimeStruct->tm_min) + FILE_TIME_DELIM_CHAR + 
				std::to_string(TimeStruct->tm_sec));
		}

	private:
		inline static std::atomic<uint64_t> FileNameIncrementor;

		inline static std::vector<std::string> UE_GetRequestBuffer;
		inline static std::vector<std::string> UE_SetRequestBuffer;
		
		inline static std::vector<std::string> AWS_SetRequestBuffer;
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
#undef GET_RESPONSE_REQUEST_STRING
#undef SET_REQUEST_STRING		
#undef FILE_DELIM_CHAR
#undef FILE_TIME_DELIM_CHAR

#undef ATTRIBUTE_CHAR_MAX

#undef UE_BUFFER_MAX
#undef AWS_BUFFER_MAX

#undef UE_BUFFER_TICK_RATE
#undef AWS_BUFFER_TICK_RATE

#undef IPC_PLATFORM_CACHE_LINE_SIZE
#undef IPC_ALIGN_TO_CACHE_LINE

#endif
