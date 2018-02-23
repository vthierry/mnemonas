#include "mnemonas.hpp"

network::Transform::Transform(unsigned int N, const Input& input) : Input(N, input.T), input(&input) {}
void network::Transform::setInput(const Input& input_)
{
  const Input **i = const_cast < const Input ** > (&input);
  *i = &input_;
}
