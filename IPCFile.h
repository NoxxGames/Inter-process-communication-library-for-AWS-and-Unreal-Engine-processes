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
#include <functional>
#include <immintrin.h>

#if defined(_WIN64) || defined(_WIN32) // windows
	#if !defined(FORCEINLINE)
		#define FORCEINLINE __forceinline
	#endif
	#define SPIN_LOOP_PAUSE _mm_pause
#else // linux
	#if !defined(FORCEINLINE)
		#define FORCEINLINE inline 
	#endif
	#define SPIN_LOOP_PAUSE __builtin_ia32_pause
#endif

#define NEWLINE_CHAR					'\n'
#define DELIM_CHAR						','
#define WRITE_MODE						"w"
#define READ_MODE						"r"
#define FILE_EXTENSION					".ipcf"
#define ATTRIBUTE_DELIM_CHAR			':'
#define TRUE_STRING						"1"
#define FALSE_STRING					"0"
#define NULL_STRING						"NULL"

#define REQUEST_ID_DELIM_CHAR			'-'
#define GET_REQUEST_STRING				"GET"
#define GET_RESPONSE_REQUEST_STRING		"GETRESPONSE"
#define SET_REQUEST_STRING				"SET"
#define FILE_DELIM_CHAR					'#'
#define FILE_TIME_DELIM_CHAR			'-'
#define FILE_FOOTER_STRING				"EOF"
#define FILE_DIRECTORY_DELIM			"\\"

#define ATTRIBUTE_CHAR_MAX				1024

#define UE_BUFFER_MAX					65536
#define AWS_BUFFER_MAX					65536
#define PENDING_REQUEST_RESERVE_SIZE	8192

#define UE_BUFFER_TICK_RATE				8
#define AWS_BUFFER_TICK_RATE			8

#define IPC_PLATFORM_CACHE_LINE_SIZE	64
#define IPC_ALIGN_TO_CACHE_LINE			alignas(IPC_PLATFORM_CACHE_LINE_SIZE)

#define SPIN_LOOP_SLEEP_TIME_MS			10

namespace IPCFile
{
	/*
	 * TODO
	 */
	enum class EAttributeTypes : uint8_t
	{
		NONE,
		STRING,
		INT,
		FLOAT,
		BOOL
	};

	/*
	 * TODO
	 */
	enum class EAttributeName : uint8_t
	{
		NONE,
		PLAYER_AUTH,
		PLAYER_NAME,
		IS_ONLINE
	};

	/*
	 * TODO
	 */
	enum class ERequestType : uint8_t
	{
		GET,
		GET_RESPONSE,
		SET
	};

	/*
	 * TODO
	 */
	struct FToken
	{
		FToken()
			: Token(GenerateNewToken())
		{
		}
		
		FToken(const FToken& InToken)
			: Token(InToken.GetToken())
		{
		}

		FToken(const uint64_t InToken)
			: Token(InToken)
		{
		}

		/*
		 * TODO
		 */
		void operator=(const FToken& In) noexcept
		{
			Token = In.GetToken();
		}

		/*
		 * TODO
		 */
		void operator=(const uint64_t In) noexcept
		{
			Token = In;
		}

		/*
		 * TODO
		 */
		FORCEINLINE uint64_t GetToken() const noexcept
		{
			return Token;
		}

		/*
		 * TODO
		 */
		FORCEINLINE void SetToken(const FToken& InToken) noexcept
		{
			Token = InToken.GetToken();
		}

		/*
		 * TODO
		 */
		FORCEINLINE void SetToken(const uint64_t InToken) noexcept
		{
			Token = InToken;
		}

		/*
		 * TODO
		 */
		FORCEINLINE std::string ToString() const noexcept
		{
			return std::to_string(Token);
		}

		/*
		 * TODO
		 */
		static FORCEINLINE void GenerateNewToken(FToken& OutToken) noexcept
		{
			OutToken.SetToken(Incrementor.fetch_add(1, std::memory_order_seq_cst));
		}
		
		/*
		 * TODO
		 */
		static FORCEINLINE uint64_t GenerateNewToken() noexcept
		{
			return Incrementor.fetch_add(1, std::memory_order_seq_cst);
		}
	
	private:
		uint64_t Token;
		inline static std::atomic<uint64_t> Incrementor = {0};
	};
	
	/*
	 * TODO
	 */
	struct FAttributeStringPair
	{
		std::string KeyString;
		std::string ValueString;
	};
	
	/*
	 * TODO
	 */
	template<typename T, EAttributeTypes TAttributeType>
	class IPC_ALIGN_TO_CACHE_LINE IAttribute
	{
		// TODO: add static_asserts on constructibility of template T
		
	public:
		static constexpr EAttributeTypes Type = TAttributeType;
		EAttributeName Name;
		T Value;
	
		IAttribute()
			: Name(EAttributeName::NONE),
			Value(T())
		{
		}
		
		IAttribute(const EAttributeName& InName,
			const T& InValue)
			: Name(InName),
			Value(InValue)
		{
		}

		FORCEINLINE EAttributeName& GetName() 
		{
			return Name;
		}

		FORCEINLINE T& GetValue()
		{
			return Value;
		}

		FORCEINLINE void SetName(const EAttributeName& InName)
		{
			Name = InName;
		}

		FORCEINLINE void SetValue(const T& InValue)
		{
			Value = InValue;
		}
	};

	class IAttributeString : public IAttribute<std::string, EAttributeTypes::STRING>
	{
	public:
		IAttributeString()
			: IAttribute<std::string, EAttributeTypes::STRING>()
		{
		}
		
		IAttributeString(const EAttributeName& InName, const std::string& InValue)
			: IAttribute<std::string, EAttributeTypes::STRING>(InName, InValue)
		{
		}
	};

	// typedef IAttribute<std::string> IAttributeString;
	typedef IAttribute<int, EAttributeTypes::INT>			IAttributeInt;
	typedef IAttribute<float, EAttributeTypes::FLOAT>		IAttributeFloat;
	typedef IAttribute<bool, EAttributeTypes::BOOL>			IAttributeBool;
	
	namespace TableDataStatics
	{
		namespace Internal
		{
			/*
			 * TODO
			 */
			template<typename T, EAttributeTypes TAttributeType>
			class IPC_ALIGN_TO_CACHE_LINE IColumnAttribute final
				: public IAttribute<T, TAttributeType>
			{
			public:
				T Key = IAttribute<T, TAttributeType>::Value;
				
				IColumnAttribute()
					: IAttribute<T, TAttributeType>()
				{
				}

				IColumnAttribute(const EAttributeName& InName,
									const T& InKey)
					: IAttribute<T, TAttributeType>(InName, InKey)
				{
				}
			};

			typedef IColumnAttribute<std::string, EAttributeTypes::STRING> IColumnAttributeString;
		}
		
		static constexpr int NumberOfAttributes = 3;
		
		static const Internal::IColumnAttributeString TableKey_PlayerAuthID =
			Internal::IColumnAttributeString(
				EAttributeName::PLAYER_AUTH,
				std::string("PlayerAuthID"));
		
		static const Internal::IColumnAttributeString TableKey_PlayerName =
			Internal::IColumnAttributeString(
				EAttributeName::PLAYER_NAME,
				std::string("PlayerName"));
		
		static const Internal::IColumnAttributeString TableKey_IsOnline =
			Internal::IColumnAttributeString(
				EAttributeName::IS_ONLINE,
				std::string("IsOnline"));
	}
	
	/*
	 * TODO
	 */
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

		/*
		 * TODO
		 */
		EAttributeName& operator[](const int Index)
		{
			return AttributesInUse[Index];
		}
		
		/*
		 * TODO
		 */
		EAttributeName operator[](const int Index) const
		{
			return AttributesInUse[Index];
		}
		
		/*
		 * TODO
		 */
		FORCEINLINE void SetPlayerAuthID(const IAttributeString& InPlayerAuthID)
		{
			if(!CheckIfAttributeIsAlreadySet(InPlayerAuthID.Name))
			{
				AttributesInUse.push_back(InPlayerAuthID.Name);
			}
			PlayerAuthID = InPlayerAuthID;
		}

		/*
		 * TODO
		 */
		FORCEINLINE void SetPlayerName(const IAttributeString& InPlayerName)
		{
			if(!CheckIfAttributeIsAlreadySet(InPlayerName.Name))
			{
				AttributesInUse.push_back(InPlayerName.Name);
			}
			PlayerName = InPlayerName;
		}

		/*
		 * TODO
		 */
		FORCEINLINE void SetIsOnline(const IAttributeBool& InIsOnline)
		{
			if(!CheckIfAttributeIsAlreadySet(InIsOnline.Name))
			{
				AttributesInUse.push_back(InIsOnline.Name);
			}
			IsOnline = InIsOnline;
		}

		/*
		 * TODO
		 */
		FORCEINLINE IAttributeString GetPlayerAuthID() const noexcept
		{
			return PlayerAuthID;
		}

		/*
		 * TODO
		 */
		FORCEINLINE IAttributeString GetPlayerName() const noexcept
		{
			return PlayerName;
		}

		/*
		 * TODO
		 */
		FORCEINLINE IAttributeBool GetIsOnline() const noexcept
		{
			return IsOnline;
		}

		/*
		 * TODO
		 */
		FORCEINLINE bool IsEmpty() const noexcept
		{
			return AttributesInUse.size() == 0;
		}

		/*
		 * TODO
		 */
		FORCEINLINE size_t Size() const noexcept
		{
			return AttributesInUse.size();
		}
		
	private:
		/*
		 * TODO
		 */
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

	/*
	 * TODO
	 */
	class IPC_ALIGN_TO_CACHE_LINE FIPCRequest
	{
	public:
		static constexpr int TotalNumberOfAttributes =
			TableDataStatics::NumberOfAttributes;

		FIPCRequest() = default;
		FIPCRequest(const IAttributeString& InPlayerAuthID,
			const std::string& InRequestID)
			: PlayerAuthID(InPlayerAuthID),
			RequestID(InRequestID)
		{
		}

		virtual ~FIPCRequest() = default;
		
		/*
		 * TODO
		 */
		virtual FORCEINLINE IAttributeString GetPlayerAuthID() const noexcept
		{
			return PlayerAuthID;
		}
		
		/*
		 * TODO
		 */
		virtual FORCEINLINE std::string GetPlayerAuthIDString() const noexcept
		{
			return PlayerAuthID.Value;
		}

		virtual FORCEINLINE std::string GetRequestID() const noexcept
		{
			return RequestID;
		}
		
		virtual FORCEINLINE bool IsEmpty() const noexcept = 0;
		virtual FORCEINLINE size_t Size() const noexcept = 0;
		
	protected:
		const IAttributeString PlayerAuthID;
		const std::string RequestID;
	};

	/*
	 * GetRequest
	 */
	class IPC_ALIGN_TO_CACHE_LINE FGetRequest : public FIPCRequest
	{
	public:
		FGetRequest() = default;
		FGetRequest(const FGetRequest& InRequest)
			: FIPCRequest(InRequest.GetPlayerAuthID(), InRequest.GetRequestID())
		{
			for(int i = 0; i < InRequest.Size(); ++i)
			{
				AddAttributeToGet(InRequest[i]);
			}
		}
		
		FGetRequest(const IAttributeString& InPlayerAuthID,
			const std::string& InRequestID)
			: FIPCRequest(InPlayerAuthID, InRequestID),
			AttributesToGet{}
		{
		}

		FGetRequest(
			const IAttributeString& InPlayerAuthID,
			const std::string& InRequestID,
			const std::vector<EAttributeName>& InAttributesToGet)
				: FIPCRequest(InPlayerAuthID, InRequestID),
				AttributesToGet(InAttributesToGet)
		{
		}

		/*
		 * TODO
		 */
		EAttributeName operator[](const int Index) const
		{
			return AttributesToGet[Index];
		}

		/*
		 * TODO
		 */
		EAttributeName& operator[](const int Index)
		{
			return AttributesToGet[Index];
		}

		/*
		 * TODO
		 */
		FORCEINLINE bool AddAttributeToGet(const EAttributeName& InAttributeName)
		{
			if(CheckIfAttributeIsAlreadySet(InAttributeName))
			{
				return false;
			}

			AttributesToGet.push_back(InAttributeName);
			return true;
		}

		/*
		 * TODO
		 */
		FORCEINLINE bool GetAttribute(
			const int Index,
			EAttributeName& OutAttribute) const
		{
			if(Index < Size())
			{
				OutAttribute = AttributesToGet[Index];
				return true;
			}
			return false;
		}

		/*!
		 * @return Checks to see if there are no @link FGetRequest
		 *			stored in the array.
		 */
		virtual FORCEINLINE bool IsEmpty() const noexcept override
		{
			return Size() == 0;
		}

		/*
		 * TODO
		 */
		virtual FORCEINLINE size_t Size() const noexcept override
		{
			return AttributesToGet.size();
		}
		
	private:
		/*
		 * TODO
		 */
		FORCEINLINE bool CheckIfAttributeIsAlreadySet(
			const EAttributeName& InAttributeName) const noexcept
		{
			if(AttributesToGet.empty())
			{
				return false;
			}

			// Player Auth ID has to be set upon construction, so it's always
			// set... It's not actually in the vector tho, it's a variable
			// on the parent of this type
			if(InAttributeName == TableDataStatics::TableKey_PlayerAuthID.Name)
			{
				return true;
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
	class IPC_ALIGN_TO_CACHE_LINE FPendingGetRequest final : public FGetRequest
	{
	public:
		FPendingGetRequest() = default;
		FPendingGetRequest(const FPendingGetRequest& InRequest)
			: FGetRequest(InRequest.GetPlayerAuthID(), InRequest.GetRequestID()),
			UniqueID(InRequest.GetUniqueID()) 
		{
		}

		FPendingGetRequest(
			const FGetRequest& InRequest,
			const std::string& InUniqueID)
			: FGetRequest(InRequest.GetPlayerAuthID(), InRequest.GetRequestID()),
			UniqueID(InUniqueID)
		{
		}
		
		FPendingGetRequest(
			const IAttributeString& InPlayerAuthID,
			const std::string& InRequestID,
			const std::string& InUniqueID)
				: FGetRequest(InPlayerAuthID, InRequestID),
				UniqueID(InUniqueID)
		{
		}
		
		/*
		 * TODO
		 */
		FORCEINLINE const std::string& GetUniqueID() const
		{
			return UniqueID;
		}

		/*
		 * TODO
		 */	
		FORCEINLINE void SetUniqueID(const std::string& InUniqueID) 
		{
			UniqueID = InUniqueID;
		}
		
	private:
		std::string UniqueID;
	};
	
	/*
	 * SetRequest
	 */
	class IPC_ALIGN_TO_CACHE_LINE FSetRequest final : public FIPCRequest
	{
	public:
		FSetRequest() = default;
		FSetRequest(const FSetRequest& InRequest)
			: FIPCRequest(InRequest.GetPlayerAuthID(), InRequest.GetRequestID()),
			PlayerAttributes(InRequest.GetPlayerAttributeList())
		{
		}
		
		FSetRequest(
			const IAttributeString& InPlayerAuthID,
			const std::string& InRequestID,
			const FPlayerAttributeList& InPlayerAttributes)
				: FIPCRequest(InPlayerAuthID, InRequestID),
				PlayerAttributes(InPlayerAttributes)
		{
		}

		/*
		 * TODO
		 */
		FORCEINLINE const FPlayerAttributeList& GetPlayerAttributeList() const
		{
			return PlayerAttributes;
		}
		
		/*
		 * TODO
		 */
		virtual FORCEINLINE bool IsEmpty() const noexcept override
		{
			return Size() == 0;
		}

		/*
		 * TODO
		 */
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
		/** The tick rate for threads on the UE side to use */
		static constexpr int UE_BufferTickRateMS = 1000 / UE_BUFFER_TICK_RATE;
		/** The tick rate for threads on the AWS side to use */
		static constexpr int AWS_BufferTickRateMS = 1000 / AWS_BUFFER_TICK_RATE;
		
		/**
		 * \brief Used to identify which platform a @link FRequestBuffer will be used on.
		 */
		enum class ERequestBufferType : uint8_t
		{
			UE,
			AWS,
			BOTH
		};

		/**
 * \brief A basic Spin-lock implementation
 * \tparam bShouldUseSleep Should this SpinLoop use a sleep or not?
 */
		template<bool bShouldUseSleep = true>
		struct FSpinLoop
		{
			FSpinLoop()
				: Flag{false}
			{
			}
			
			/**
			 * \brief Run a Lambda functor through the lock
			 * \param LambdaFunctor The functor to run
			 * \return TODO this always returns true...
			 */
			FORCEINLINE void RunLambdaThroughLock(
				std::function<void()> LambdaFunctor)
			{
				Lock();
				LambdaFunctor();
				Unlock();
			}
			
			/*
			 * TODO
			 */
			FORCEINLINE void Lock() noexcept
			{
				for(;;)
				{
					if(!Flag.exchange(true, std::memory_order_acquire))
					{
						return;
					}

					while(Flag.load(std::memory_order_relaxed))
					{
						if(bShouldUseSleep)
						{
							std::this_thread::sleep_for(
								std::chrono::milliseconds(
									SPIN_LOOP_SLEEP_TIME_MS));
						}
						else
						{
							SPIN_LOOP_PAUSE();
						}
					}
				}
			}

			/*
			 * TODO
			 */
			FORCEINLINE bool TryLock() noexcept
			{
				return !Flag.load(std::memory_order_relaxed) &&
					!Flag.exchange(true, std::memory_order_acquire);
			}

			/*
			 * TODO
			 */
			FORCEINLINE void Unlock()
			{
				Flag.store(false, std::memory_order_release);
			}

		private:
			std::atomic<bool> Flag;
		};

		/**
		 * TODO This type currently is using a vector to store the data..
		 * TODO this is not very efficient memory wise.. a circular buffer
		 * TODO is required to make this simple
		 * 
		 * \brief Base type used for the @link FGetRequest and @link FSetRequest buffer types.
		 * \tparam T The type of data that will be stored in the buffer.
		 * \tparam TBufferPlatform The platform (UE/AWS) that this buffer is being used for.
		 */
		template<typename T, ERequestBufferType TBufferPlatform>
		class IPC_ALIGN_TO_CACHE_LINE FRequestBuffer
		{
		public:
			FRequestBuffer()
				: ReserveSize{(TBufferPlatform == ERequestBufferType::UE) ?
					(UE_BUFFER_MAX) : (AWS_BUFFER_MAX)},
				CurrentReserveMultiplier{1}
			{
				RequestBuffer.reserve(
					ReserveSize * CurrentReserveMultiplier);
			}
			
			/**
			 * \brief Initialize this buffer
			 */
			virtual FORCEINLINE void Initialize()
			{
				FRequestBuffer();
			}

			/**
			 * \brief Lock the buffer
			 */
			virtual FORCEINLINE void LockBuffer() noexcept
			{
				BufferLock.Lock();
			}

			/**
			 * \brief Unlock the buffer
			 */
			virtual FORCEINLINE void UnlockBuffer() noexcept
			{
				BufferLock.Unlock();
			}

			/**
			 * \brief Get a reference to the buffer
			 */
			virtual FORCEINLINE std::vector<T>& GetBuffer()
			{
				return RequestBuffer;
			}
			
			/**
			 * \brief Pushes an element into the buffer, in a thread safe manner. 
			 * \param InRequest Element to push into the buffer.
			 */
			virtual FORCEINLINE void PushBack(const T& InRequest)
			{
				const bool ShouldReserve = (Size() == (ReserveSize.load(
					std::memory_order_relaxed) * CurrentReserveMultiplier.load(
						std::memory_order_acquire) - 1));
				uint64_t Old = 0;
				if(ShouldReserve)
				{
					Old = CurrentReserveMultiplier.fetch_add(
						1, std::memory_order_acq_rel);
				}
				
				BufferLock.RunLambdaThroughLock([=]()
				{
					if(ShouldReserve)
					{
						RequestBuffer.reserve(ReserveSize * (Old + 1));
					}
					RequestBuffer.push_back(InRequest);
				});
				BufferSize.fetch_add(1, std::memory_order_acq_rel);
			}

			virtual FORCEINLINE bool RemoveIndex(const uint32_t Index)
			{
				if(IsEmpty())
				{
					return false;
				}
				BufferSize.fetch_sub(1, std::memory_order_acq_rel);
				BufferLock.RunLambdaThroughLock([=]()
				{
					// RequestBuffer.erase(Index); TODO
				});
				return true;
			}
			
			/**
			 * \brief Completely erase all elements from the buffer in a thread safe manner.
			 */
			virtual FORCEINLINE void Clear()
			{
				BufferLock.RunLambdaThroughLock([=]() -> void
				{
					RequestBuffer.clear();
					RequestBuffer.reserve(ReserveSize.load(std::memory_order_relaxed));
				});
				CurrentReserveMultiplier.store(1, std::memory_order_release);
				BufferSize.store(0, std::memory_order_release);
			}
			
			/**
			 * \brief Checks if there are no elements in the buffer in a thread safe manner.
			 * \return Whether or not the buffer is currently empty.
			 */
			virtual FORCEINLINE bool IsEmpty() const noexcept
			{
				return Size() == 0;
			}
			
			/**
			 * \brief Check how many elements are in the buffer in a thread safe manner. 
			 * \return The amount of elements currently in the buffer.
			 */
			virtual FORCEINLINE size_t Size() const noexcept
			{
				return BufferSize.load(std::memory_order_acquire);
			}
			
			/**
			 * \brief Runs a given functor that will be protected by the @link FSpinLoop lock.
			 * \param Lambda Lambda functor to be run through the lock.
			 */
			virtual FORCEINLINE void RunLambdaThroughLock(std::function<void()> Lambda)
			{
				BufferLock.RunLambdaThroughLock(Lambda);
			}
			
		protected:
			const std::atomic<uint64_t> ReserveSize;
			std::atomic<uint64_t> CurrentReserveMultiplier;
			std::atomic<uint64_t> BufferSize;

			FSpinLoop<true> BufferLock;
			mutable std::vector<T> RequestBuffer;
		};
		
		/**
		 * \brief A buffer used to store all pending @link FGetRequest while they wait to be written to a file.
		 * \tparam TBufferPlatform The platform (UE/AWS) that this buffer is being used for.
		 */
		template<ERequestBufferType TBufferPlatform>
		class IPC_ALIGN_TO_CACHE_LINE FGetRequestBuffer final
			: public FRequestBuffer<FGetRequest, TBufferPlatform>
		{
		public:
			FGetRequestBuffer()
				: FRequestBuffer<FGetRequest, TBufferPlatform>()
			{
			}

			/**
			 * \brief Initialize this buffer
			 */
			virtual FORCEINLINE void Initialize() override
			{
				FGetRequestBuffer();
			}
			
			/**
			 * \brief Write all the current @link FGetRequest in this buffer to a specified file location.
			 * \param FileLocation The directory to write the file to.
			 */
			FORCEINLINE void WriteGetRequestsToFileThroughLock(
				const std::string& FileLocation)
			{
				this->RunLambdaThroughLock([=]()
				{
					std::string CompleteFileString = "";
					for(int i = 0; i < this->Size(); ++i)
					{
						const FGetRequest Request = this->RequestBuffer[i];
						std::string CurrentLine;
						const std::string RequestID = Request.GetRequestID();
						CurrentLine.append(RequestID + REQUEST_ID_DELIM_CHAR);
						// add the player auth to the beginning so we know who it's for
						const std::string PlayerAuth = Request.GetPlayerAuthIDString() +
							DELIM_CHAR;
						CurrentLine.append(PlayerAuth);
						for(int j = 0; j < Request.Size(); ++j)
						{
							switch(Request[j])
							{
								case EAttributeName::NONE:
									break;
								case EAttributeName::PLAYER_AUTH:
									CurrentLine.append(TableKey_PlayerAuthID.Key);
									break;
								case EAttributeName::PLAYER_NAME:
									CurrentLine.append(TableKey_PlayerName.Key);
									break;
								case EAttributeName::IS_ONLINE:
									CurrentLine.append(TableKey_IsOnline.Key);
									break;
								default:
									break;
							}
							CurrentLine += DELIM_CHAR;
						}
						CurrentLine += NEWLINE_CHAR;
						CompleteFileString.append(CurrentLine);
					}
					CompleteFileString.append(FILE_FOOTER_STRING);

					std::string UniqueFileName;
					GeneratorUniqueFileName(UniqueFileName, ERequestType::GET);
					if(UniqueFileName == NULL_STRING)
					{
						return;
					}
					
					const std::string FullNameAndPath = FileLocation +
						FILE_DIRECTORY_DELIM + UniqueFileName + FILE_EXTENSION;
								
					WriteStringToFile(FullNameAndPath, CompleteFileString);
				});
			}
		};

		/**
		 * TODO
		 */
		template<ERequestBufferType TBufferPlatform>
		class IPC_ALIGN_TO_CACHE_LINE FPendingGetRequestBuffer final
			: public FRequestBuffer<FPendingGetRequest, TBufferPlatform>
		{
		public:
			FPendingGetRequestBuffer()
				: FRequestBuffer<FPendingGetRequest, TBufferPlatform>()
			{
			}

			/**
			 * \brief Initialize this buffer
			 */
			virtual FORCEINLINE void Initialize() override
			{
				FPendingGetRequestBuffer();
			}
			
			/**
			 * TODO
			 */
			FPendingGetRequest operator[](const int Index)
			{
				FPendingGetRequest Out;
				this->RunLambdaThroughLock([=]()
				{
					Out = this->RequestBuffer[Index];
				});
				return Out;
			}
			
			/**
			 * TODO
			 */
			FORCEINLINE bool RemoveElement(const uint32_t Index)
			{
				return this->RemoveIndex(Index);
			}
		};
		
		/**
		 * \brief A buffer used to store all pending @link FSetRequest while they wait to be written to a file.
		 * \tparam TBufferPlatform The platform (UE/AWS) that this buffer is being used for.
		 */
		template<ERequestBufferType TBufferPlatform>
		class IPC_ALIGN_TO_CACHE_LINE FSetRequestBuffer final
			: public FRequestBuffer<FSetRequest, TBufferPlatform>
		{
		public:
			FSetRequestBuffer()
				: FRequestBuffer<FSetRequest, TBufferPlatform>()
			{
			}

			/**
			 * \brief Initialize this buffer
			 */
			virtual FORCEINLINE void Initialize() override
			{
				FSetRequestBuffer();
			}
			
			/**
			 * \brief Write all the current @link FSetRequest in this buffer to a specified file location.
			 * \param FileLocation The directory to write the file to.
			 */
			FORCEINLINE void WriteSetRequestsToFileThroughLock(
				const std::string& FileLocation)
			{
				this->RunLambdaThroughLock([=]()
				{
					std::string CompleteFileString = "";
					for(int i = 0; i < this->Size(); ++i)
					{
						const FSetRequest Request = this->RequestBuffer[i];
						const FPlayerAttributeList PlayerAttributes =
							Request.GetPlayerAttributeList();
						std::string CurrentLine = "";
						
						for(int j = 0; j < PlayerAttributes.Size(); ++j)
						{
							// Combine each attribute key and value into a string
							// then append it to the line.
							FAttributeStringPair StringPair;
							for(int k = 0; k < PlayerAttributes.Size(); ++k)
							{
								switch(PlayerAttributes[k])
								{
									case EAttributeName::NONE:
										// it should never be this...
										break;
									case EAttributeName::PLAYER_AUTH:
										StringPair.KeyString = TableKey_PlayerAuthID.Key;
										StringPair.ValueString =
											PlayerAttributes.GetPlayerAuthID().Value;
										break;
									case EAttributeName::PLAYER_NAME:
										StringPair.KeyString = TableKey_PlayerName.Key;
										StringPair.ValueString =
											PlayerAttributes.GetPlayerName().Value;
										break;
									case EAttributeName::IS_ONLINE:
										StringPair.KeyString = TableKey_IsOnline.Key;
										StringPair.ValueString =
											(PlayerAttributes.GetIsOnline().Value) ?
												(TRUE_STRING) : (FALSE_STRING);
										break;
									default:
										// should not be able to get here...
										break;
								}
								const std::string AttributeString = StringPair.KeyString +
									ATTRIBUTE_DELIM_CHAR + StringPair.ValueString + DELIM_CHAR;
								CurrentLine.append(AttributeString);
							}
						}
						CurrentLine += NEWLINE_CHAR;
						CompleteFileString.append(CurrentLine);
					}
					CompleteFileString.append(FILE_FOOTER_STRING);

					std::string UniqueFileName;
					GeneratorUniqueFileName(UniqueFileName, ERequestType::SET);
					if(UniqueFileName == NULL_STRING)
					{
						return;
					}
					const std::string FullNameAndPath = FileLocation +
						FILE_DIRECTORY_DELIM + UniqueFileName + FILE_EXTENSION;
						
					WriteStringToFile(FullNameAndPath, CompleteFileString);
				});
			}
		};
		
		/*
		 * TODO Need to make sure any requests in the buffers are written to file
		 * TODO upon shutdown of the threads & erasing the buffers...
		 * TODO ... obivous data loss otherwise
		 *
		 * TODO
		 */
		template<ERequestBufferType TBufferRequestType>
		class IPC_ALIGN_TO_CACHE_LINE FBufferThread
		{
			static constexpr int TickRate =
				(TBufferRequestType == ERequestBufferType::UE) ?
					(UE_BufferTickRateMS) : (AWS_BufferTickRateMS);
			
		public:
			FBufferThread()
				: IsRunning{false},
				ShouldStop{false}
			{
			}
			
			/**
			 * \brief Start the thread
			 * \param Functor The functor that represents one tick of this thread.
			 */
			virtual FORCEINLINE void StartThread(
				const std::function<void()>& Functor)
			{
				if(IsRunning.load(std::memory_order_acquire))
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
						
						Functor();
						std::this_thread::sleep_for(
							std::chrono::milliseconds(TickRate));
					}

					IsRunning.store(false, std::memory_order_release);
				}).detach();
			}

			/**
			 * \brief Stop the thread from running.
			 */
			virtual FORCEINLINE void StopThread()
			{
				ShouldStop.store(true, std::memory_order_release);
			}

			/*
			 * \brief Check whether or not the thread is currently running.
			 */
			virtual FORCEINLINE bool GetIsRunning() const
			{
				return IsRunning.load(std::memory_order_acquire);
			}

		private:
			std::atomic<bool> IsRunning;
			std::atomic<bool> ShouldStop;
		};
		
	public:
		template<typename T, EAttributeTypes TAttributeType> using FColumnAttribute	=
			TableDataStatics::Internal::IColumnAttribute<T, TAttributeType>;
		
		inline static const FColumnAttribute<std::string, EAttributeTypes::STRING>
			TableKey_PlayerAuthID = TableDataStatics::TableKey_PlayerAuthID;
		inline static const FColumnAttribute<std::string, EAttributeTypes::STRING>
			TableKey_PlayerName = TableDataStatics::TableKey_PlayerName;
		inline static const FColumnAttribute<std::string, EAttributeTypes::STRING>
			TableKey_IsOnline = TableDataStatics::TableKey_IsOnline;

		template<ERequestBufferType TBufferRequestType> using FWriteBufferThread =
			FBufferThread<TBufferRequestType>;
		template<ERequestBufferType TBufferRequestType> using FReadBufferThread =
			FBufferThread<TBufferRequestType>;
		
		IPCFileManager() = default;
		~IPCFileManager() = default;


		
		/* 
		 * The AWS Process will never need to make a get request to the
		 * unreal process, the AWS process will only make a set request to
		 * the unreal process, after the unreal process makes a get request
		 *
		 * UE get request -> AWS Process -> Loads from DynamoDB ->
		 * AWS set request -> UE Process
		 *
		 * UE set request -> AWS Process -> Set in DynamoDB
		 */
		
		/*
		 * #################################################
		 * ############### UNREAL FUNCTIONS ################
		 * #################################################
		 */
		
		/**
		 * \brief Initialize the system on the UE side
		 */
		static FORCEINLINE void UE_Initialize()
		{
			Initialize();
			UE_GetRequestBuffer.Initialize();
			UE_GetWriteThread.StartThread([=]()
			{
				// TODO
			});
			
			UE_SetRequestBuffer.Initialize();
			UE_SetWriteThread.StartThread([=]()
			{
				// TODO
			});

			UE_SetReadThread.StartThread([=]()
			{
				// TODO
			});

			UE_GetPendingRequestsBuffer.Initialize();
			UE_GetPendingThread.StartThread([=]()
			{
				// TODO
			});
		}
		
		/**
		 * \brief Shutdown the threads and erase all buffers on the UE side
		 */
		static FORCEINLINE void UE_Shutdown()
		{
			UE_SetWriteThread.StopThread();
			UE_SetReadThread.StopThread();
			UE_GetWriteThread.StopThread();
			UE_GetPendingThread.StopThread();
			
			// Wait for all threads to shutdown before erasing the buffers...
			while(UE_SetWriteThread.GetIsRunning() ||
				UE_SetReadThread.GetIsRunning() ||
				UE_GetWriteThread.GetIsRunning() ||
				UE_GetPendingThread.GetIsRunning())
			{
				std::this_thread::sleep_for(
					std::chrono::milliseconds(10));
			}
			UE_GetRequestBuffer.Clear();
			UE_SetRequestBuffer.Clear();
			UE_GetPendingRequestsBuffer.Clear();
			Shutdown();
		}

		/**
		 * \brief Add a @link FGetRequest to the buffer
		 * \param GetRequest The @link FGetRequest to add to the buffer
		 * \return Whether or not the Add worked
		 */
		static FORCEINLINE bool UE_AddGetRequestToBuffer(
			const FGetRequest& GetRequest)
		{
			if(GetRequest.IsEmpty())
			{
				return false;
			}
			UE_GetRequestBuffer.PushBack(GetRequest);
			UE_GetPendingRequestsBuffer.PushBack(
				FPendingGetRequest(GetRequest, GetRequest.GetRequestID()));
			return true;
		} 
		
		/**
		 * \brief Add a @link FSetRequest to the buffer
		 * \param SetRequest The @link FSetRequest to add to the buffer
		 * \return Whether or not the Add worked
		 */
		static FORCEINLINE bool UE_AddSetRequestToBuffer(
			const FSetRequest& SetRequest)
		{
			if(SetRequest.IsEmpty())
			{
				return false;
			}
			UE_SetRequestBuffer.PushBack(SetRequest);
			return true;
		}
		
		/**
		 * \brief Write the entire @link FGetRequestBuffer to file
		 * \param FileLocation The directory to put the file into
		 * \return Whether or not the write worked
		 */
		static FORCEINLINE bool UE_WriteGetRequestBufferToFile(
			const std::string& FileLocation)
		{
			if(UE_GetRequestBuffer.IsEmpty())
			{
				return false;
			}
			
			UE_GetRequestBuffer.WriteGetRequestsToFileThroughLock(
				FileLocation);
			return true;
		}

		/**
		 * \brief Write the entire @link FSetRequestBuffer to file
		 * \param FileLocation The directory to put the file into
		 * \return Whether or not the write worked
		 */
		static FORCEINLINE void UE_WriteSetRequestBufferToFile(
			const std::string& FileLocation)
		{
			if(UE_SetRequestBuffer.IsEmpty())
			{
				return;
			}

			UE_SetRequestBuffer.WriteSetRequestsToFileThroughLock(
				FileLocation);
		}

		/*
		 * #################################################
		 * ################# AWS FUNCTIONS #################
		 * #################################################
		 */
		
		/**
		 * \brief Initialize the system on the AWS side
		 */
		static FORCEINLINE void AWS_Initialize()
		{
			AWS_SetRequestBuffer.Initialize();
			AWS_SetWriteThread.StartThread([=]()
			{
				// TODO
			});

			AWS_SetReadThread.StartThread([=]()
			{
				// TODO
			});
			
			AWS_GetReadThread.StartThread([=]()
			{
				// TODO
			});
		}

		/*
		 * \brief Shutdown all threads and erase all bufers on the AWS side
		 */
		static FORCEINLINE void AWS_Shutdown()
		{
			AWS_SetWriteThread.StopThread();
			AWS_SetReadThread.StopThread();
			AWS_GetReadThread.StopThread();
			
			// Wait for all threads to shutdown before erasing the buffers...
			while(AWS_SetWriteThread.GetIsRunning() ||
				AWS_SetReadThread.GetIsRunning() ||
				AWS_GetReadThread.GetIsRunning())
			{
				std::this_thread::sleep_for(
					std::chrono::milliseconds(10));
			}
			AWS_SetRequestBuffer.Clear();
			Shutdown();
		}
		
		/**
		 * \brief Add a @link FSetRequest to the buffer
		 * \param SetRequest The @link FSetRequest to add to the buffer
		 * \return Whether or not the Add worked
		 */
		static FORCEINLINE bool AWS_AddSetRequestToBuffer(
			const FSetRequest& SetRequest)
		{
			if(SetRequest.IsEmpty())
			{
				return false;
			}
			AWS_SetRequestBuffer.PushBack(SetRequest);
			return true;
		}

		/**
		 * \brief Write the entire @link FSetRequestBuffer to file
		 * \param FileLocation The directory to put the file into
		 * \return Whether or not the write worked
		 */
		static FORCEINLINE void AWS_WriteSetRequestBufferToFile(
			const std::string& FileLocation)
		{
			if(AWS_SetRequestBuffer.IsEmpty())
			{
				return;
			}
			AWS_SetRequestBuffer.WriteSetRequestsToFileThroughLock(FileLocation);
		}
		
		/*
		 * TODO
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

		/*
		 * TODO
		 */
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

		/**
		 * \brief Create a unique ID for a @link FIPCRequest
		 */
		static FORCEINLINE std::string GenerateUniqueRequestID() noexcept
		{
			const FToken UniqueIDToken = FToken::GenerateNewToken();
			const std::string UniqueIDString = UniqueIDToken.ToString();
			return UniqueIDString;
		}
		
	private:
		static FORCEINLINE void Initialize()
		{
		}

		static FORCEINLINE void Shutdown()
		{
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

		/*
		 * TODO
		 */
		static FORCEINLINE bool WriteStringToFile(
			const std::string& FullNameAndPath,
			const std::string& FileString)
		{
			FILE *File;
			fopen_s(&File, FullNameAndPath.c_str(), WRITE_MODE);
			if(!File)
			{
				return false;
			}
			fputs(FileString.c_str(), File);
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

		/*
		 * TODO
		 */
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

		/*
		 * TODO
		 */
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

		/*
		 * TODO
		 */
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
						EAttributeName::PLAYER_AUTH,
						KeyValueStringPair.ValueString);
					PlayerAttributes.SetPlayerAuthID(Buffer);
				}
				else if(KeyValueStringPair.KeyString == TableKey_PlayerName.Key)
				{
					const IAttributeString Buffer(
						EAttributeName::PLAYER_NAME,
						KeyValueStringPair.ValueString);
					PlayerAttributes.SetPlayerName(Buffer);
				}
				else if(KeyValueStringPair.KeyString == TableKey_IsOnline.Key)
				{
					const IAttributeBool Buffer(
						EAttributeName::IS_ONLINE,
						(KeyValueStringPair.ValueString == TRUE_STRING));
					PlayerAttributes.SetIsOnline(Buffer);
				}
			}
		}
		
		/**
		 * \brief Get a list of files in a particular directory
		 * \param Directory The directory to search for files in
		 * \param OutFileList The vector that the file names will be stored in
		 * \return Fails if the directory does not exist, or if there were no files in the directory
		 */
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
		
		/**
		 * \brief Create a unique file name
		 * \param Out The string to store the generated name in
		 * \param RequestType The type of request the file name is for
		 */
		static FORCEINLINE void GeneratorUniqueFileName(std::string& Out,
			const ERequestType& RequestType)
		{
			const FToken UniqueIDToken = FToken::GenerateNewToken();
			const std::string UniqueIDString = UniqueIDToken.ToString();
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
			const std::string SystemTime = GetSystemTimeAsString();
			if(SystemTime == NULL_STRING)
			{
				Out = NULL_STRING;
				return;
			}
			// Output the file name
			Out = RequestTypeString + FILE_DELIM_CHAR + UniqueIDString +
				FILE_DELIM_CHAR + SystemTime;
		}
		
		/**
		 * \brief Get the system time as a @link std::string
		 * \return An @link std::string that represents the current local time.
		 */
		static FORCEINLINE std::string GetSystemTimeAsString() noexcept
		{
			time_t CurrentTime;
			tm *TimeStruct = new tm();
			time(&CurrentTime);
			localtime_s(TimeStruct, &CurrentTime);
			std::string Time = NULL_STRING;
			if(TimeStruct)
			{
				Time = std::to_string(TimeStruct->tm_hour) + FILE_TIME_DELIM_CHAR + 
					std::to_string(TimeStruct->tm_min) + FILE_TIME_DELIM_CHAR + 
					std::to_string(TimeStruct->tm_sec);
			}
			delete TimeStruct;
			return Time;
		}

	private:
		inline static FGetRequestBuffer			<ERequestBufferType::UE>		UE_GetRequestBuffer;
		inline static FWriteBufferThread		<ERequestBufferType::UE>		UE_GetWriteThread;

		inline static FSetRequestBuffer			<ERequestBufferType::UE>		UE_SetRequestBuffer;
		inline static FWriteBufferThread		<ERequestBufferType::UE>		UE_SetWriteThread;
		inline static FReadBufferThread			<ERequestBufferType::UE>		UE_SetReadThread;

		inline static FPendingGetRequestBuffer	<ERequestBufferType::UE>		UE_GetPendingRequestsBuffer;
		inline static FReadBufferThread			<ERequestBufferType::UE>		UE_GetPendingThread;
		
		inline static FSetRequestBuffer			<ERequestBufferType::AWS>		AWS_SetRequestBuffer;
		inline static FWriteBufferThread		<ERequestBufferType::AWS>		AWS_SetWriteThread;
		inline static FReadBufferThread			<ERequestBufferType::AWS>		AWS_SetReadThread;
		inline static FReadBufferThread			<ERequestBufferType::AWS>		AWS_GetReadThread;
	};
}

#undef SPIN_LOOP_PAUSE

#undef NEWLINE_CHAR
#undef DELIM_CHAR
#undef WRITE_MODE
#undef READ_MODE
#undef FILE_EXTENSION
#undef ATTRIBUTE_DELIM_CHAR
#undef TRUE_STRING
#undef FALSE_STRING
#undef NULL_STRING

#undef REQUEST_ID_DELIM_CHAR
#undef GET_REQUEST_STRING
#undef GET_RESPONSE_REQUEST_STRING
#undef SET_REQUEST_STRING		
#undef FILE_DELIM_CHAR
#undef FILE_TIME_DELIM_CHAR
#undef FILE_FOOTER_STRING
#undef FILE_DIRECTORY_DELIM

#undef ATTRIBUTE_CHAR_MAX

#undef UE_BUFFER_MAX
#undef AWS_BUFFER_MAX
#undef PENDING_REQUEST_RESERVE_SIZE

#undef UE_BUFFER_TICK_RATE
#undef AWS_BUFFER_TICK_RATE

#undef IPC_PLATFORM_CACHE_LINE_SIZE
#undef IPC_ALIGN_TO_CACHE_LINE

#undef SPIN_LOOP_SLEEP_TIME_MS

#endif
