
#include "pipeline.hpp"


class scene_graph {};

class p1 : public rge::pass<scene_graph, p1>
{
public:
	p1(scene_graph* input) : rge::pass<scene_graph, p1>(input) {}

};

class p2_frame {};
class p2_frame_holder : public rge::basic_frame_holder<p2_frame>
{
	void prepare_and_submit_frame_segment(p2_frame& frame) override
	{

	}
};

class p2 : public rge::pass<p1, p2, p2_frame_holder> {
public:
	p2(p1* input) : rge::pass<p1, p2, p2_frame_holder>(input) {}
};

class p3_frame {};
class p3_frame_holder : public rge::basic_frame_holder<p3_frame>
{
	void prepare_and_submit_frame_segment(p3_frame& frame) override
	{

	}
};

class p3 : public rge::pass<p2, p3, p3_frame_holder> {
public:
	p3(p2* input) : rge::pass<p2, p3, p3_frame_holder>(input) {}
};


rge::pipeline<p1, p2, p3> test_pipeline(new scene_graph());

