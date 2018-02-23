namespace network {
  /** Defines an input/output transform. */
  class Transform: public Input {
public:
    /** Gets the transform input. */
    const Input *const input;

    /** Resets the transform for the given parameters.
     * @param N The number of output units, defined unit indexes stand in <tt>{0, N{</tt>.
     * @param input The transform input.
     */
    Transform(unsigned int N, const Input &input);
    /** Sets the transform input. */
    virtual void setInput(const Input& input);
  };
}
