
#pragma once

#include "scene/packet_handler.hpp"

#include <stdexcept>
#include <vector>
#include <tuple>
#include <concepts>
#include <type_traits>
#include <utility>

namespace rge
{
	// ------------------------------------------------------------------------------------------------ pipeline

	class base_pass {};

	template<typename _input, typename _output, typename... _components> requires
		std::is_pointer_v<_input> &&
		std::is_pointer_v<_output>
	class pass : public base_pass
	{
	public:
		using input_t = _input;
		using output_t = _output;
		using components_t = std::tuple<_components...>;

	private:
		components_t m_components;

	public:
		input_t m_input;
		output_t m_output;

		pass(input_t input) :
			m_input(input)
		{}

		template<typename _component>
		_component& get_component()
		{
			return std::get<_component>(m_components);
		}
	};

	template<typename _pass>
	concept is_pass = std::is_base_of_v<base_pass, _pass>;

	template<is_pass... _passes>
	class pipeline
	{
	public:
		static constexpr size_t s_passes_count = sizeof...(_passes);
		static constexpr size_t s_last_pass_idx = s_passes_count - 1;

		template<size_t _idx>
		using get_pass_t = typename std::tuple_element<_idx, std::tuple<_passes...>>::type; // Access by idx: slow approach.

		template<size_t _idx>
		using get_pass_input_t = typename get_pass_t<_idx>::input_t;

		template<size_t _idx>
		using get_pass_output_t = typename get_pass_t<_idx>::output_t;

	private:
		std::array<std::unique_ptr<base_pass>, s_passes_count> m_passes;

	public:
		template<typename _input, typename _output>
		struct combinable_pass
		{
			using input_t = _input;
			using output_t = _output;

			template<typename _other_output>
			combinable_pass<_input, _other_output> operator|(combinable_pass<_output, _other_output>)
			{}
		};

		template<typename _pass>
		using make_combinable_pass = combinable_pass<typename _pass::input_t, typename _pass::output_t>;

		using combined_t = decltype((make_combinable_pass<_passes>{} | ...)); // This is a way to check that the passes' input/output match.

		using input_t = typename combined_t::input_t;
		using output_t = typename combined_t::output_t;

		template<size_t _idx>
		get_pass_t<_idx>* get_pass()
		{
			return static_cast<get_pass_t<_idx>*>(m_passes[_idx]);
		}

		template<size_t _idx>
		get_pass_input_t<_idx>* get_pass_input()
		{
			return get_pass<_idx>()->m_input;
		}

		template<size_t _idx>
		get_pass_output_t<_idx>* get_pass_output()
		{
			return get_pass<_idx>()->m_output;
		}

		input_t get_input()
		{
			return get_pass_input<0>();
		}

		output_t get_output()
		{
			return get_pass_output<s_last_pass_idx>();
		}

		template<size_t _idx>
		get_pass_t<_idx>* concat_pipeline(get_pass_input_t<_idx>* input)
		{
			auto pass = new get_pass_t<_idx>(input);

			m_passes[_idx] = std::unique_ptr<base_pass>(pass);

			if constexpr(_idx + 1 < s_passes_count)
			{
				auto output = pass->m_output;
				concat_pipeline<_idx + 1>(static_cast<get_pass_input_t<_idx + 1>*>(output));
			}

			return pass;
		}

		pipeline(input_t* input)
		{
			concat_pipeline<0>(input);
		}

		~pipeline()
		{}
	};

	// ------------------------------------------------------------------------------------------------ frame_holder

	struct frame_holder
	{
		virtual void recreate_frame_segments(size_t count) = 0;
		virtual void prepare_and_submit_frame_segment(size_t idx) = 0;
	};

	template<typename _frame_segment>
	struct basic_frame_holder : frame_holder
	{
		std::vector<_frame_segment> m_frame_segments;

		void recreate_frame_segments(size_t count) override
		{
			m_frame_segments.resize(count);
		};

		virtual void prepare_and_submit_frame_segment(_frame_segment& frame_segment) = 0;

		void prepare_and_submit_frame_segment(size_t idx) override
		{
			prepare_and_submit_frame_segment(m_frame_segments.at(idx));
		}
	};

	template<is_pass... _passes>
	class renderer_pipeline : public pipeline<_passes...>
	{
	private:
		using pipeline_t = pipeline<_passes...>;

	public:
		template<typename _component, typename _f, typename... _args> requires std::is_invocable_v<_f, _component, _args...>
		void call_func_on_component(_f func, _args&& ... args)
		{
			auto per_pass = [func, ...args = std::forward<_args>(args)]<
				size_t _idx,
				template<typename, typename, typename...> typename _pass, typename... _components
			>()
			{
				if constexpr((std::is_base_of_v<_component, _components> | ...)) {
					auto pass = pipeline_t::template get_pass<_idx>();
					auto component = pass->template get_component<_component>();

					std::invoke(func, component, args...);
				}
			};

			[per_pass]<size_t... _is>(std::index_sequence<_is...>){
				(per_pass.template operator()<_is, _passes>(), ...);
			}
			(std::index_sequence_for<_passes...>());
		}

		void recreate_frame_segments(size_t count)
		{
			auto per_frame_holder = [count](rge::frame_holder& frame_holder) {
				frame_holder.recreate_frame_segments(count);
			};

			call_func_on_component<rge::frame_holder>(per_frame_holder);
		}

		void prepare_and_submit_frame_segment(size_t idx)
		{
			auto per_frame_holder = [idx](rge::frame_holder& frame_holder) {
				frame_holder.prepare_and_submit_frame_segment(idx);
			};

			call_func_on_component<rge::frame_holder>(per_frame_holder);
		}
	};
}
