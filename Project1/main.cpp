#include <iostream>
#include <tuple>

template<typename T, int N>
struct property_counter { 
	friend constexpr void adl_flag(property_counter<T, N>); 
};

template<typename T, int N>
struct writer {
	friend constexpr void adl_flag(property_counter<T, N>) { }
	static constexpr int value = N;
};

template<typename T, int N, typename = std::enable_if_t<noexcept(adl_flag(property_counter<T, N>()))>>
int constexpr reader(int, property_counter<T, N>) {
	return N;
}

template<typename T, int N>
int constexpr reader(float, property_counter<T, N>, int R = reader(0, property_counter<T, N - 1>())) {
	return R;
}

template <typename T>
int constexpr reader(float, property_counter<T, 0>) {
	return 0;
}

template<typename T, int N = 1, int C = reader(0, property_counter<T, 32>())>
int constexpr count_property(int R = C) {
	writer<T, C + N>();
	return R;
}

template <typename T>
int constexpr property_count() {
	return reader(0, property_counter<T, 32>());
}

template <typename GET, typename SET>
class property
{
private:
	std::string m_key;
	GET m_getter;
	SET m_setter;
public:
	property(const std::string& key, GET getter, SET setter) :
		m_key(key), m_getter(getter), m_setter(setter) {}
};

template <typename ...>
struct tuple_concat {};

template <typename... L, typename... R>
struct tuple_concat<std::tuple<L...>, std::tuple<R...>>
{
	using type = std::tuple<L..., R...>;
};

struct IEntity
{
	using this_t = IEntity;

	template <typename T>
	using member_ptr = T this_t::*;

	template <size_t I, typename...>
	struct property_field {};

	template <>
	struct property_field<count_property<this_t>()>
	{
		const member_ptr<std::string> member = &this_t::m_name;
	};
	using properties = std::tuple<property_field<0>>;

	std::string m_name;

	static properties GetProperties()
	{
		property_count<this_t>();
		return std::make_tuple(property_field<0>());
	}
};

struct Character : public IEntity
{
	using parent_t = this_t;
	using this_t = Character;
	
	template <typename T>
	using member_ptr = T this_t::*;

	writer<this_t, reader(0, property_counter<parent_t, 32>())> temp;
};

#define REGISTER_ENTITY(entity) \
protected: \
	using parent_t = this_t; \
	using this_t = entity; \
	writer<this_t, reader(0, property_counter<parent_t, 32>())> temp; \
public: \
	template <size_t N> \
	static void Property() { parent_t::Property<N>(); } \

#define REGISTER_FIELD(member) \
	template <> \
	static void Property<count_property<this_t>()>() { std::cout << #member << "\n"; }

struct base_struct
{
protected:
	using this_t = base_struct;
public:
	template <size_t>
	static void Property() { std::cout << "default\n"; }

	REGISTER_FIELD(member_0)
};

struct derived_struct : public base_struct
{
	REGISTER_ENTITY(derived_struct)

	REGISTER_FIELD(member_1)


};

struct derived_struct1 : public derived_struct
{
	REGISTER_ENTITY(derived_struct1)
	
	REGISTER_FIELD(member_2)
	REGISTER_FIELD(member_3)
};

void main()
{
	derived_struct::Property<0>();
	derived_struct::Property<1>();
	derived_struct::Property<2>();

	derived_struct1::Property<0>();
	derived_struct1::Property<1>();
	derived_struct1::Property<2>();
	derived_struct1::Property<3>();
	derived_struct1::Property<4>();
}