#ifndef APPLICATION_H
#define APPLICATION_H
class application
{
 public:
  virtual void run(void) = 0;
  static application* factory(void);
};
#endif//APPLICATION_H
