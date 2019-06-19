#include <iostream>
#include <tuple>
#include <functional>
#include <utility>
#include <string>

template<typename T, int N>
struct property_counter { friend constexpr void adl_flag(property_counter<T, N>); };

template<typename T, int N>
struct writer {
	friend constexpr void adl_flag(property_counter<T, N>) { }
	static constexpr int value = N;
};

template<typename T, int N, typename = std::enable_if_t<noexcept(adl_flag(property_counter<T, N>()))>>
int constexpr reader(int, property_counter<T, N>) { return N; }

template<typename T, int N>
int constexpr reader(float, property_counter<T, N>, int R = reader(0, property_counter<T, N - 1>())) { return R; }

template <typename T>
int constexpr reader(float, property_counter<T, 0>) { return 0; }

template<typename T, int N = 1, int C = reader(0, property_counter<T, 32>())>
int constexpr count_property(int R = C) { writer<T, C + N>(); return R; }

template <typename T>
int constexpr property_count() { return reader(0, property_counter<T, 32>()); }

template <typename ...>
struct tuple_concat {};
template <typename... L, typename... R>
struct tuple_concat<std::tuple<L...>, std::tuple<R...>> { using type = std::tuple<L..., R...>; };
template <typename... L, typename R>
struct tuple_concat<std::tuple<L...>, R> { using type = std::tuple<L..., R>; };
template <typename L, typename R>
using tuple_concat_t = typename tuple_concat<L, R>::type;

template <class C, typename T>
using member_ptr_t = T C::*;
template <class C, typename T>
using const_ref_const_getter_ptr_t = const T& (C::*)() const;
template <class C, typename T>
using const_ref_getter_ptr_t = const T& (C::*)();
template <class C, typename T>
using const_val_getter_ptr_t = const T(C::*)();
template <class C, typename T>
using val_getter_ptr_t = T(C::*)();
template <class C, typename T>
using const_ref_setter_ptr_t = void (C::*)(const T&);
template <class C, typename T>
using ref_setter_ptr_t = void (C::*)(T&);
template <class C, typename T>
using const_val_setter_ptr_t = void (C::*)(const T);
template <class C, typename T>
using val_setter_ptr_t = void (C::*)(T);

template <
	class Class, typename T,
	template<class, typename> class Get,
	template<class, typename> class Set>
struct property_field
{
	constexpr property_field(const char* name, Get<Class, T> getter, Set<Class, T> setter) :
		m_name(name), m_getter(getter), m_setter(setter) {}

	const char* name() const { return m_name; }
	T get(Class& object) const { return (object.*m_getter)(); }
	void set(Class& object, T value) const { (object.*m_setter)(value); }

	const char* m_name;
	Get<Class, T> m_getter;
	Set<Class, T> m_setter;
};

template <class Class, typename T>
auto constexpr create_property_field(const char* name, T(Class::* getter)(), void (Class::* setter)(T))
{
	return property_field<Class, T, val_getter_ptr_t, val_setter_ptr_t>(name, getter, setter);
}

struct IEntity
{
	using this_t = IEntity;

	template <size_t I>
	struct property_element {};

	std::string m_name;
	std::string GetName() { return m_name; }
	void SetName(std::string name) { m_name; }
	template <>
	struct property_element<0>
	{
		static constexpr const auto field = create_property_field("NAME", &this_t::GetName, &this_t::SetName);
	};

	int m_speed;
	int GetSpeed() { return m_speed; }
	void SetSpeed(int speed) { m_speed = speed; }
	template<>
	struct property_element<1> : public property_element<1 - 1>
	{
		static constexpr const auto field = create_property_field("SPEED", &this_t::GetSpeed, &this_t::SetSpeed);
	};
};

struct Character : public IEntity
{
	using parent_t = this_t;
	using this_t = Character;

	template <size_t I, typename...>
	struct property_element : public parent_t::property_element<I> {};
	int m_hp;
	int GetHP() { return m_hp; }
	void SetHP(int hp) { m_hp = hp; }
	template <>
	struct property_element<2> : public property_element<2 - 1>
	{
		static constexpr const auto field = create_property_field("HP", &this_t::GetHP, &this_t::SetHP);
	};
	static property_element<2> GetProperties() { return property_element<2>(); }
};

struct Item : public IEntity
{
	using parent_t = this_t;
	using this_t = Item;

	template <size_t I, typename...>
	struct property_element : public parent_t::property_element<I> {};
	float m_multiplier;
	float GetMultiplier() { return m_multiplier; }
	void SetMultiplier(float hp) { m_multiplier = hp; }
	template <>
	struct property_element<2> : public property_element<2 - 1>
	{
		static constexpr const auto field = create_property_field("MULTIPLIER", &this_t::GetMultiplier, &this_t::SetMultiplier);
	};
	static property_element<2> GetProperties() { return property_element<2>(); }
};

template <class C, template<size_t> class T, size_t N, size_t I = 0>
typename std::enable_if < N < I, void>::type
	PrintProperties(C& object, const T<N>& properties) {}

template <class C, template<size_t> class T, size_t N, size_t I = 0>
typename std::enable_if<I <= N, void>::type
PrintProperties(C& object, const T<N>& properties)
{
	std::cout
		<< T<I>::field.name() << ": "
		<< T<I>::field.get(object) << '\n';
	PrintProperties<C, T, N, I + 1>(object, properties);
}

void main()
{
	Character testentity;
	testentity.m_name = "fsa";
	testentity.m_speed = 1;
	testentity.m_hp = 5;
	PrintProperties(testentity, Character::GetProperties());

	Item testentity1;
	testentity1.m_name = "gwed";
	testentity1.m_speed = 2;
	testentity1.m_multiplier = 6.4;
	PrintProperties(testentity1, Item::GetProperties());
}