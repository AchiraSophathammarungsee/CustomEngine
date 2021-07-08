#ifndef RECT_H
#define RECT_H

template<typename T>
struct Rect
{
public:
  Rect() : TopLeft_X(0), TopLeft_Y(0), BotRight_X(0), BotRight_Y(0), Scale_X(0), Scale_Y(0) {};
  Rect(const T& topleft_x, const T& topleft_y, const T& botright_x, const T& botright_y) : TopLeft_X(topleft_x), TopLeft_Y(topleft_y), BotRight_X(botright_x), BotRight_Y(botright_y) 
  {
    CalcScale();
  };
  ~Rect() {};

  void CalcScale() 
  {
    Scale_X = BotRight_X - TopLeft_X;
    Scale_Y = BotRight_Y - TopLeft_Y;
  };

  bool IsInRect(const T& x, const T& y) const
  {
    return x >= TopLeft_X && x <= BotRight_X && y <= BotRight_Y && y >= TopLeft_Y;
  }

  T TopLeftX() const { return TopLeft_X; };
  T TopLeftY() const { return TopLeft_Y; };
  T BotRightX() const { return BotRight_X; };
  T BotRightY() const { return BotRight_Y; };
  T ScaleX() const { return Scale_X; };
  T ScaleY() const { return Scale_Y; };
  
private:
  T TopLeft_X, TopLeft_Y, BotRight_X, BotRight_Y, Scale_X, Scale_Y;
};

#endif