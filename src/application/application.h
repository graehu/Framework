#ifndef APPLICATION_H
#define APPLICATION_H
class application
{
 public:
  virtual void mf_run(void) = 0;
  static application* mf_factory(void);
};
#endif//APPLICATION_H
