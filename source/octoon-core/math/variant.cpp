#include <octoon/math/variant.h>

namespace octoon::math
{
	Variant::Variant()
		: type_(Type::Void)
	{
		value_.i = 0;
	}

	Variant::Variant(bool rhs)
		: type_(Type::Bool)
	{
		value_.b = rhs;
	}

	Variant::Variant(int rhs)
		: type_(Type::Int)
	{
		value_.i = rhs;
	}

	Variant::Variant(float rhs)
		: type_(Type::Float)
	{
		value_.f[0] = rhs;
	}

	Variant::Variant(const char* chrPtr)
		: type_(Type::String)
	{
		value_.string = new std::string(chrPtr);
	}

	Variant::Variant(const void* ptr)
		: type_(Type::Object)
	{
		value_.object = ptr;
	}

	Variant::Variant(const float4& rhs)
		: type_(Type::Float4)
	{
		value_.f[0] = rhs.x;
		value_.f[1] = rhs.y;
		value_.f[2] = rhs.z;
		value_.f[3] = rhs.w;
	}

	Variant::Variant(const std::string& rhs)
		:type_(Type::String)
	{
		value_.string = new std::string(rhs);
	}

	Variant::Variant(const Quaternion& rhs)
		: type_(Type::Quaternion)
	{
		value_.quaternion = rhs;
	}

	Variant::Variant(const float4x4& rhs)
		: type_(Type::Float4x4)
	{
		value_.matrix = new float4x4(rhs);
	}

	Variant::Variant(const std::vector<int>& rhs)
		: type_(Type::IntArray)
	{
		value_.intArray = new std::vector<int>(rhs);
	}

	Variant::Variant(const std::vector<float>& rhs)
		: type_(Type::FloatArray)
	{
		value_.floatArray = new std::vector<float>(rhs);
	}

	Variant::Variant(const std::vector<bool>& rhs)
		: type_(Type::BoolArray)
	{
		value_.boolArray = new std::vector<bool>(rhs);
	}

	Variant::Variant(const std::vector<float4>& rhs)
		: type_(Type::Float4Array)
	{
		value_.float4Array = new std::vector<float4>(rhs);
	}

	Variant::Variant(const std::vector<float4x4>& rhs)
		: type_(Type::Float4x4Array)
	{
		value_.float4x4Array = new std::vector<float4x4>(rhs);
	}

	Variant::Variant(const std::vector<std::string>& rhs)
		: type_(Type::StringArray)
	{
		value_.stringArray = new std::vector<std::string>(rhs);
	}

	Variant::Variant(const Variant& rhs)
		: type_(rhs.type_)
	{
		this->setVariant(rhs);
	}

	Variant::~Variant()
	{
		this->destroy();
	}

	void
	Variant::setType(Type t)
	{
		type_ = t;

		switch (t)
		{
		case Type::Void:
			this->destroy();
			break;
		case Type::String:
			this->destroy();
			value_.string = new std::string;
			break;
		case Type::Float4x4:
			this->destroy();
			value_.matrix = new float4x4;
			break;
		case Type::Object:
			this->destroy();
			value_.object = nullptr;
			break;
		case Type::IntArray:
			this->destroy();
			value_.intArray = new std::vector<int>;
			break;
		case Type::FloatArray:
			this->destroy();
			value_.floatArray = new std::vector<float>;
			break;
		case Type::BoolArray:
			this->destroy();
			value_.boolArray = new std::vector<bool>;
			break;
		case Type::Float4Array:
			this->destroy();
			value_.float4Array = new std::vector<float4>;
			break;
		case Type::Float4x4Array:
			this->destroy();
			value_.float4x4Array = new std::vector<float4x4>;
			break;
		case Type::StringArray:
			this->destroy();
			value_.stringArray = new std::vector<std::string>;
			break;
		default:
			break;
		}
	}

	Variant::Type
	Variant::getType() const
	{
		return type_;
	}

	void
	Variant::setInt(int val)
	{
		assert(type_ == Type::Int);
		value_.i = val;
	}

	void
	Variant::setFloat(float val)
	{
		assert(type_ == Type::Float);
		value_.f[0] = val;
	}

	void
	Variant::setBool(bool val)
	{
		assert(type_ == Type::Bool);
		value_.b = val;
	}

	void
	Variant::setString(const std::string& val)
	{
		assert(type_ == Type::String);
		*value_.string = val;
	}

	void
	Variant::setFloat4(const float4& val)
	{
		assert(type_ == Type::Float4);
		value_.f[0] = val.x;
		value_.f[1] = val.y;
		value_.f[2] = val.z;
		value_.f[3] = val.w;
	}

	void
	Variant::setFloat4x4(const float4x4& val)
	{
		assert(type_ == Type::Float4x4);
		*value_.matrix = val;
	}

	void
	Variant::setQuaternion(const Quaternion& val)
	{
		assert(type_ == Type::Quaternion);
		value_.quaternion = val;
	}

	void
	Variant::setObject(const void* ptr)
	{
		assert(type_ == Type::Object);
		value_.object = ptr;
	}

	void
	Variant::setBoolArray(const std::vector<bool>& val)
	{
		assert(type_ == Type::BoolArray);
		*value_.boolArray = val;
	}

	void
	Variant::setIntArray(const std::vector<int>& val)
	{
		assert(type_ == Type::IntArray);
		*value_.intArray = val;
	}

	void
	Variant::setFloatArray(const std::vector<float>& val)
	{
		assert(type_ == Type::FloatArray);
		*value_.floatArray = val;
	}

	void
	Variant::setFloat4Array(const std::vector<float4>& val)
	{
		assert(type_ == Type::Float4Array);
		*value_.float4Array = val;
	}

	void
	Variant::setFloat4x4Array(const std::vector<float4x4>& val)
	{
		assert(type_ == Type::Float4x4Array);
		*value_.float4x4Array = val;
	}

	void
	Variant::setStringArray(const std::vector<std::string>& val)
	{
		assert(type_ == Type::StringArray);
		*value_.stringArray = val;
	}

	void
	Variant::setVariant(const Variant& rhs)
	{
		assert(type_ == rhs.type_);

		type_ = rhs.type_;

		switch (rhs.type_)
		{
		case Type::Void:
			break;
		case Type::Bool:
			value_.b = rhs.value_.b;
			break;
		case Type::Int:
			value_.i = rhs.value_.i;
			break;
		case Type::Float:
			value_.f[0] = rhs.value_.f[0];
			break;
		case Type::Float4:
			value_.f[0] = rhs.value_.f[0];
			value_.f[1] = rhs.value_.f[1];
			value_.f[2] = rhs.value_.f[2];
			value_.f[3] = rhs.value_.f[3];
			break;
		case Type::Quaternion:
			value_.quaternion = rhs.value_.quaternion;
			break;
		case Type::Object:
			value_.object = rhs.value_.object;
			break;
		case Type::String:
			*value_.string = *rhs.value_.string;
			break;
		case Type::Float4x4:
			*value_.matrix = *value_.matrix;
			break;
		case Type::IntArray:
			*value_.intArray = *value_.intArray;
			break;
		case Type::FloatArray:
			*value_.floatArray = *value_.floatArray;
			break;
		case Type::BoolArray:
			*value_.boolArray = *value_.boolArray;
			break;
		case Type::Float4Array:
			*value_.float4Array = *value_.float4Array;
			break;
		case Type::Float4x4Array:
			*value_.float4x4Array = *value_.float4x4Array;
			break;
		case Type::StringArray:
			*value_.stringArray = *value_.stringArray;
			break;
		default:
			throw std::runtime_error("Variant::copy(): invalid type!");
			break;
		}
	}

	int
	Variant::getInt() const
	{
		assert(Type::Int == type_);
		return value_.i;
	}

	float
	Variant::getFloat() const
	{
		assert(Type::Float == type_);
		return value_.f[0];
	}

	bool
	Variant::getBool() const
	{
		assert(Type::Bool == type_);
		return value_.b;
	}

	const std::string&
	Variant::getString() const
	{
		assert(Type::String == type_);
		return *(value_.string);
	}

	const float4&
	Variant::getFloat4() const
	{
		assert(Type::Float4 == type_);
		return (float4&)value_.f;
	}

	const math::Quaternion&
	Variant::getQuaternion() const
	{
		assert(Type::Quaternion == type_);
		return value_.quaternion;
	}

	const float4x4&
	Variant::getFloat4x4() const
	{
		assert(Type::Float4x4 == type_);
		return *(value_.matrix);
	}

	const void*
	Variant::getObject() const
	{
		assert(Type::Object == type_);
		return value_.object;
	}

	const std::vector<int>&
	Variant::getIntArray() const
	{
		assert(Type::IntArray == type_);
		return *(value_.intArray);
	}

	const std::vector<float>&
	Variant::getFloatArray() const
	{
		assert(Type::FloatArray == type_);
		return *(value_.floatArray);
	}

	const std::vector<bool>&
	Variant::getBoolArray() const
	{
		assert(Type::BoolArray == type_);
		return *(value_.boolArray);
	}

	const std::vector<float4>&
	Variant::getFloat4Array() const
	{
		assert(Type::Float4Array == type_);
		return *(value_.float4Array);
	}

	const std::vector<float4x4>&
	Variant::getFloat4x4Array() const
	{
		assert(Type::Float4x4Array == type_);
		return *(value_.float4x4Array);
	}

	const std::vector<std::string>&
	Variant::getStringArray() const
	{
		assert(Type::StringArray == type_);
		return *(value_.stringArray);
	}

	std::string
	Variant::to_string(Type t)
	{
		switch (t)
		{
		case Type::Void:          return "void";
		case Type::Int:           return "int";
		case Type::Float:         return "float";
		case Type::Bool:          return "bool";
		case Type::Float4:        return "float4";
		case Type::String:        return "string";
		case Type::Float4x4:      return "float4x4";
		case Type::Blob:          return "blob";
		case Type::Guid:          return "guid";
		case Type::Object:        return "object";
		case Type::IntArray:      return "intarray";
		case Type::FloatArray:    return "floatarray";
		case Type::BoolArray:     return "boolarray";
		case Type::Float4Array:   return "float4array";
		case Type::Float4x4Array: return "float4x4Array";
		case Type::StringArray:   return "stringarray";
		default:
			throw std::runtime_error("Variant::typeToString(): invalid type enum!");
		}
	}

	Variant::Type
	Variant::to_type(const std::string& str)
	{
		if ("void" == str)             return Type::Void;
		else if ("int" == str)              return Type::Int;
		else if ("float" == str)            return Type::Float;
		else if ("bool" == str)             return Type::Bool;
		else if ("float4" == str)           return Type::Float4;
		else if ("color" == str)            return Type::Float4; // NOT A BUG!
		else if ("string" == str)           return Type::String;
		else if ("Float4x4" == str)        return Type::Float4x4;
		else if ("blob" == str)             return Type::Blob;
		else if ("guid" == str)             return Type::Guid;
		else if ("object" == str)           return Type::Object;
		else if ("intarray" == str)         return Type::IntArray;
		else if ("floatarray" == str)       return Type::FloatArray;
		else if ("boolarray" == str)        return Type::BoolArray;
		else if ("float4array" == str)      return Type::Float4Array;
		else if ("Float4x4array" == str)   return Type::Float4x4Array;
		else if ("stringarray" == str)      return Type::StringArray;
		else
		{
			throw std::runtime_error("Variant::stringToType(): invalid type string!");
		}
	}

	void
	Variant::destroy()
	{
		if (Type::String == type_)
		{
			assert(value_.string);
			delete value_.string;
			value_.string = 0;
		}
		else if (Type::Float4x4 == type_)
		{
			assert(value_.matrix);
			delete value_.matrix;
			value_.matrix = 0;
		}
		else if (Type::Object == type_)
		{
			if (value_.object)
			{
				//_value.object->Release();
				value_.object = 0;
			}
		}
		else if (Type::IntArray == type_)
		{
			assert(value_.intArray);
			delete value_.intArray;
			value_.intArray = 0;
		}
		else if (Type::FloatArray == type_)
		{
			assert(value_.floatArray);
			delete value_.floatArray;
			value_.floatArray = 0;
		}
		else if (Type::BoolArray == type_)
		{
			assert(value_.boolArray);
			delete value_.boolArray;
			value_.boolArray = 0;
		}
		else if (Type::Float4Array == type_)
		{
			assert(value_.float4Array);
			delete value_.float4Array;
			value_.float4Array = 0;
		}
		else if (Type::Float4x4Array == type_)
		{
			assert(value_.float4x4Array);
			delete value_.float4x4Array;
			value_.float4x4Array = 0;
		}
		else if (Type::StringArray == type_)
		{
			assert(value_.stringArray);
			delete value_.stringArray;
			value_.stringArray = 0;
		}

		type_ = Type::Void;
	}
}