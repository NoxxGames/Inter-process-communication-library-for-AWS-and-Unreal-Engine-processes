#ifndef IPC_FILE_H
#define IPC_FILE_H

#include <fstream>
#include <string>
#include <vector>

#ifndef FORCEINLINE
	#define FORCEINLINE __forceinline //windows
#endif

namespace IPCFile
{
	enum class EAttributeTypes : uint8_t
	{
		NONE,
		STRING,
		INT,
		FLOAT,
	};

	template<typename T>
	struct FAttribute
	{
		EAttributeTypes Type;
		T Value;

		FAttribute()
			: Type(EAttributeTypes::NONE),
			Value(T())
		{
		}
		
		FAttribute(const EAttributeTypes& InType, const T& InValue)
			: Type(InType),
			Value(InValue)
		{
		}
	};

	typedef FAttribute<std::string> FAttributeString;
	typedef FAttribute<int>			FAttributeInt;
	typedef FAttribute<float>		FAttributeFloat;
	
	struct FTableDataStatics
	{
		static constexpr int NumberOfAttributes = 2;
		
		static constexpr FAttributeString TableKeyPlayerAuthID =
			FAttributeString(EAttributeTypes::STRING, "PlayerID");
		static constexpr FAttributeString TableKeyPlayerName =
			FAttributeString(EAttributeTypes::STRING, "PlayerName");
		
	};

	struct FPlayerAttributeList
	{
		static constexpr int NumberOfAttributes = 2;

		FAttributeString PlayerAuthID;
		FAttributeString PlayerName;

		FPlayerAttributeList()
			: PlayerAuthID(),
			PlayerName()
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

			std::string* StringBuffer = GetFileData(FileName, Directory);

			free(StringBuffer);
		}
		
	protected:
		/**
		 * \brief Convert the value of an attribute to a string.
		 * \tparam T Template type for the attribute type.
		 * \param Attribute Attribute to stringify.
		 * \param Out String to store the output in.
		 */
		template<typename T>
		static FORCEINLINE void StringifyAttribute(const FAttribute<T>& Attribute,
													std::string& Out)
		{
			// TODO switch on EAttributeTypes variable in FAttribute
			// to check which type we need to convert
		}
		
		static FORCEINLINE void GetSystemTimeAsString(std::string& Out)
		{
			// TODO
		}

		static FORCEINLINE void  GetSystemTime()
		{
			// TODO
		}
		
	private:
		static FORCEINLINE std::string* GetFileData(const std::string& FileName,
													const std::string& Directory)
		{
			std::string* StringBuffer = (std::string*)malloc(
				sizeof(std::string));

			return StringBuffer;
		}

		static FORCEINLINE FPlayerAttributeList CreateAttributeList(
												const std::string& AttributeString)
		{
			return FPlayerAttributeList();
		}
	};
	
}

#endif
