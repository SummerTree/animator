#ifndef OCTOON_VARIANT_H_
#define OCTOON_VARIANT_H_

#include <octoon/math/mat4.h>
#include <octoon/runtime/platform.h>
#include <stdexcept>

namespace octoon::math
{
	class OCTOON_EXPORT Variant final
	{
	public:
		enum class Type
		{
			Void,
			Int,
			Float,
			Bool,
			Float4,
			Quaternion,
			String,
			Float4x4,
			Blob,
			Guid,
			Object,
			IntArray,
			FloatArray,
			BoolArray,
			Float4Array,
			StringArray,
			Float4x4Array,
		};

		Variant();
		Variant(int rhs);
		Variant(float rhs);
		Variant(bool rhs);
		Variant(const char* chrPtr);
		Variant(const void* ptr);
		Variant(const float4& v);
		Variant(const float4x4& m);
		Variant(const Quaternion& val);
		Variant(const std::string& rhs);
		Variant(const std::vector<int>& rhs);
		Variant(const std::vector<float>& rhs);
		Variant(const std::vector<bool>& rhs);
		Variant(const std::vector<float4>& rhs);
		Variant(const std::vector<float4x4>& rhs);
		Variant(const std::vector<std::string>& rhs);
		Variant(const Variant& rhs);

		~Variant();

		void setType(Type t);
		Type getType() const;
		
		void destroy();

		void assign(int val) { this->setInt(val); }
		void assign(float val) { this->setFloat(val); }
		void assign(bool val) { this->setBool(val); }
		void assign(const void* ptr) { this->setObject(ptr); }
		void assign(const float4& val) { this->setFloat4(val); }
		void assign(const float4x4& val) { this->setFloat4x4(val); }
		void assign(const Quaternion& val) { this->setQuaternion(val); }
		void assign(const std::string& val) { this->setString(val); }
		void assign(const std::vector<int>& val) { this->setIntArray(val); }
		void assign(const std::vector<float>& val) { this->setFloatArray(val); }
		void assign(const std::vector<bool>& val) { this->setBoolArray(val); }
		void assign(const std::vector<float4>& val) { this->setFloat4Array(val); }
		void assign(const std::vector<float4x4>& val) { this->setFloat4x4Array(val); }
		void assign(const std::vector<std::string>& val) { this->setStringArray(val); }
		void assign(const Variant& variant) { this->setVariant(variant); }

		void setInt(int val);
		void setFloat(float val);
		void setBool(bool val);
		void setFloat4(const float4& val);
		void setFloat4x4(const float4x4& val);
		void setQuaternion(const Quaternion& val);
		void setObject(const void* ptr);
		void setString(const std::string& val);
		void setIntArray(const std::vector<int>& val);
		void setFloatArray(const std::vector<float>& val);
		void setBoolArray(const std::vector<bool>& val);
		void setFloat4Array(const std::vector<float4>& val);
		void setFloat4x4Array(const std::vector<float4x4>& val);
		void setStringArray(const std::vector<std::string>& val);
		void setVariant(const Variant& rhs);

		int getInt() const;
		float getFloat() const;
		bool getBool() const;
		const std::string& getString() const;
		const float4& getFloat4() const;
		const Quaternion& getQuaternion() const;
		const float4x4& getFloat4x4() const;
		const void* getObject() const;
		const std::vector<int>& getIntArray() const;
		const std::vector<float>& getFloatArray() const;
		const std::vector<bool>& getBoolArray() const;
		const std::vector<float4>& getFloat4Array() const;
		const std::vector<float4x4>& getFloat4x4Array() const;
		const std::vector<std::string>& getStringArray() const;

		static std::string to_string(Type t);
		static Type to_type(const std::string& str);

		void operator=(bool val) { this->assign(val); }
		void operator=(int val) { this->assign(val); }
		void operator=(float val) { this->assign(val); }
		void operator=(const float4& val) { this->assign(val); }
		void operator=(const math::Quaternion& val) { this->assign(val); }
		void operator=(const std::string& val) { this->assign(val); }
		void operator=(const float4x4& val) { this->assign(val); }
		void operator=(const void* val) { this->assign(val); }
		void operator=(const std::vector<int>& val) { this->assign(val); }
		void operator=(const std::vector<float>& val) { this->assign(val); }
		void operator=(const std::vector<bool>& val) { this->assign(val); }
		void operator=(const std::vector<float4>& val) { this->assign(val); }
		void operator=(const std::vector<float4x4>& val) { this->assign(val); }
		void operator=(const std::vector<std::string>& val) { this->assign(val); }
		void operator=(const Variant& val) { this->assign(val); }

	public:
		friend bool operator==(const Variant& lhs, int rhs)
		{
			return (lhs.type_ == Type::Int && lhs.value_.i == rhs);
		}

		friend bool operator==(const Variant& lhs, float rhs)
		{
			return (lhs.type_ == Type::Float && lhs.value_.f[0] == rhs);
		}

		friend bool operator==(const Variant& lhs, bool rhs)
		{
			return (lhs.type_ == Type::Bool && lhs.value_.b == rhs);
		}

		friend bool operator==(const Variant& lhs, const void* ptr)
		{
			return (lhs.type_ == Type::Object && lhs.value_.object == ptr);
		}

		friend bool operator==(const Variant& lhs, const float4& rhs)
		{
			return (lhs.type_ == Type::Float4 || math::equal(lhs.value_.f[0], rhs.x) || math::equal(lhs.value_.f[1], rhs.y) || math::equal(lhs.value_.f[2], rhs.z) || math::equal(lhs.value_.f[3], rhs.w));
		}

		friend bool operator==(const Variant& lhs, const math::Quaternion& rhs)
		{
			return (lhs.type_ == Type::Quaternion && lhs.value_.quaternion == rhs);
		}

		friend bool operator==(const Variant& lhs, const std::string& rhs)
		{
			return (lhs.type_ == Type::String && *lhs.value_.string == rhs);
		}

		friend bool operator==(const Variant& lhs, const Variant& rhs)
		{
			if (lhs.type_ == rhs.type_)
			{
				switch (lhs.type_)
				{
				case Type::Void:
					return true;
				case Type::Int:
					return (lhs.value_.i == rhs.value_.i);
				case Type::Bool:
					return (lhs.value_.b == rhs.value_.b);
				case Type::Float:
					return (lhs.value_.f[0] == rhs.value_.f[0]);
				case Type::Quaternion:
					return (lhs.value_.quaternion == rhs.value_.quaternion);
				case Type::String:
					return ((*lhs.value_.string) == (*rhs.value_.string));
				case Type::Float4:
					return ((lhs.value_.f[0] == rhs.value_.f[0]) && (lhs.value_.f[1] == rhs.value_.f[1]) && (lhs.value_.f[2] == rhs.value_.f[2]) && (lhs.value_.f[3] == rhs.value_.f[3]));
				case Type::Object:
					return (lhs.value_.object == rhs.value_.object);
				case Type::Float4x4:
					return lhs.value_.matrix == rhs.value_.matrix;
				default:
					throw std::runtime_error("Variant::operator==(): invalid variant type!");
				}
			}

			return false;
		}

		friend bool operator!=(const Variant& lhs, int rhs)
		{
			return (lhs.type_ != Type::Int || lhs.value_.i != rhs);
		}

		friend bool operator!=(const Variant& lhs, float rhs)
		{
			return (lhs.type_ != Type::Float || lhs.value_.f[0] != rhs);
		}

		friend bool operator!=(const Variant& lhs, bool rhs)
		{
			return (lhs.type_ != Type::Bool || lhs.value_.b != rhs);
		}

		friend bool operator!=(const Variant& lhs, const void* ptr)
		{
			return (lhs.type_ != Type::Object || lhs.value_.object != ptr);
		}

		friend bool operator!=(const Variant& lhs, const std::string& rhs)
		{
			return (lhs.type_ != Type::String || *lhs.value_.string != rhs);
		}

		friend bool operator!=(const Variant& lhs, const float4& rhs)
		{
			return (lhs.type_ != Type::Float4 || !math::equal(lhs.value_.f[0], rhs.x) || !math::equal(lhs.value_.f[1], rhs.y) || !math::equal(lhs.value_.f[2], rhs.z) || !math::equal(lhs.value_.f[3], rhs.w));
		}

		friend bool operator!=(const Variant& lhs, const math::Quaternion& rhs)
		{
			return (lhs.type_ != Type::Quaternion || lhs.value_.quaternion != rhs);
		}

		friend bool operator!=(const Variant& lhs, const Variant& rhs)
		{
			return !(lhs == rhs);
		}

		friend bool operator>(const Variant& lhs, const Variant& rhs)
		{
			if (rhs.type_ == lhs.type_)
			{
				switch (rhs.type_)
				{
				case Type::Void:
					return true;
				case Type::Int:
					return (lhs.value_.i > rhs.value_.i);
				case Type::Bool:
					return (lhs.value_.b > rhs.value_.b);
				case Type::Float:
					return (lhs.value_.f[0] > rhs.value_.f[0]);
				case Type::String:
					return ((*lhs.value_.string) > (*rhs.value_.string));
				case Type::Float4:
					return ((lhs.value_.f[0] > rhs.value_.f[0]) &&
						(lhs.value_.f[1] > rhs.value_.f[1]) &&
						(lhs.value_.f[2] > rhs.value_.f[2]) &&
						(lhs.value_.f[3] > rhs.value_.f[3]));
				case Type::Object:
					return (lhs.value_.object > rhs.value_.object);
				default:
					throw std::runtime_error("Variant::operator>(): invalid variant type!");
				}
			}

			return false;
		}

		friend bool operator<(const Variant& lhs, const Variant& rhs)
		{
			if (rhs.type_ == lhs.type_)
			{
				switch (rhs.type_)
				{
				case Type::Void:
					return true;
				case Type::Int:
					return (lhs.value_.i < rhs.value_.i);
				case Type::Bool:
					return (lhs.value_.b < rhs.value_.b);
				case Type::Float:
					return (lhs.value_.f[0] < rhs.value_.f[0]);
				case Type::String:
					return ((*lhs.value_.string) < (*rhs.value_.string));
				case Type::Float4:
					return ((lhs.value_.f[0] < rhs.value_.f[0]) &&
						(lhs.value_.f[1] < rhs.value_.f[1]) &&
						(lhs.value_.f[2] < rhs.value_.f[2]) &&
						(lhs.value_.f[3] < rhs.value_.f[3]));
				case Type::Object:
					return (lhs.value_.object < rhs.value_.object);
				default:
					throw std::runtime_error("Variant::operator<(): invalid variant type!");
				}
			}

			return false;
		}

		friend bool operator>=(const Variant& lhs, const Variant& rhs)
		{
			if (rhs.type_ == lhs.type_)
			{
				switch (rhs.type_)
				{
				case Type::Void:
					return true;
				case Type::Int:
					return (lhs.value_.i >= rhs.value_.i);
				case Type::Bool:
					return (lhs.value_.b >= rhs.value_.b);
				case Type::Float:
					return (lhs.value_.f[0] >= rhs.value_.f[0]);
				case Type::String:
					return ((*lhs.value_.string) >= (*rhs.value_.string));
				case Type::Float4:
					return ((lhs.value_.f[0] >= rhs.value_.f[0]) &&
						(lhs.value_.f[1] >= rhs.value_.f[1]) &&
						(lhs.value_.f[2] >= rhs.value_.f[2]) &&
						(lhs.value_.f[3] >= rhs.value_.f[3]));
				case Type::Object:
					return (lhs.value_.object >= rhs.value_.object);
				default:
					throw std::runtime_error("Variant::operator>(): invalid variant type!");
				}
			}

			return false;
		}

		friend bool operator<=(const Variant& lhs, const Variant& rhs)
		{
			if (rhs.type_ == lhs.type_)
			{
				switch (rhs.type_)
				{
				case Type::Void:
					return true;
				case Type::Int:
					return (lhs.value_.i <= rhs.value_.i);
				case Type::Bool:
					return (lhs.value_.b <= rhs.value_.b);
				case Type::Float:
					return (lhs.value_.f[0] <= rhs.value_.f[0]);
				case Type::String:
					return ((*lhs.value_.string) <= (*rhs.value_.string));
				case Type::Float4:
					return ((lhs.value_.f[0] <= rhs.value_.f[0]) &&
						(lhs.value_.f[1] <= rhs.value_.f[1]) &&
						(lhs.value_.f[2] <= rhs.value_.f[2]) &&
						(lhs.value_.f[3] <= rhs.value_.f[3]));
				case Type::Object:
					return (lhs.value_.object <= rhs.value_.object);
				default:
					throw std::runtime_error("Variant::operator<(): invalid variant type!");
				}
			}

			return false;
		}

		friend Variant operator+(const Variant& lhs, const Variant& rhs) noexcept(false)
		{
			assert(lhs.type_ == rhs.type_);

			switch (lhs.type_)
			{
			case Type::Int:
				return Variant(lhs.value_.i * rhs.value_.i);
			case Type::Float:
				return Variant(lhs.value_.f.x * rhs.value_.f.x);
			case Type::Float4:
				return Variant(lhs.value_.f * rhs.value_.f);
			case Type::Quaternion:
				return Variant(lhs.value_.quaternion + rhs.value_.quaternion);
			default:
				throw std::runtime_error("Variant::operator+(): invalid variant type!");
			}
		}

		friend Variant operator*(const Variant& v, float value) noexcept(false)
		{
			switch (v.type_)
			{
			case Type::Int:
				return Variant(v.value_.i * value);
			case Type::Float:
				return Variant(v.value_.f.x * value);
			case Type::Float4:
				return Variant(v.value_.f * value);
			case Type::Quaternion:
				return Variant(v.value_.quaternion * value);
			default:
				throw std::runtime_error("Variant::operator*(): invalid variant type!");
			}
		}

	private:
		Type type_;

		union
		{
			int i;
			bool b;
			const void* object;
			float4 f;
			float4x4* matrix;
			Quaternion quaternion;
			std::string* string;
			std::vector<int>* intArray;
			std::vector<float>* floatArray;
			std::vector<bool>* boolArray;
			std::vector<float4>* float4Array;
			std::vector<float4x4>* float4x4Array;
			std::vector<std::string>* stringArray;
		} value_;
	};
}

#endif