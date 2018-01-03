namespace network {
  /** Defines an input/output transform. */
  class Transform: public Input {
protected:
    const Input& input;
public:
    /** Resets the transform for the given parameters.
     * @param N The number of output units, defined unit indexes stand in <tt>{0, N{</tt>.
     * @param input The transform input.
     */
    Transform(unsigned int N, const Input &input) : Input(N, input.getT()), input(input) {}
    /** Gets the transform input. */
    const Input& getInput() const
    {
      return input;
    }
  };
}
