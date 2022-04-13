using System;

namespace Dc
{
    public static class Mathf
    {
        public const float PI = (float)Math.PI;
        public const float TwoPI = (float)(Math.PI * 2.0);

        public const float Deg2Rad = PI / 180.0f;
        public const float Rad2Deg = 180.0f / PI;

        public static float Sin(float value) => (float)Math.Sin(value);
        public static float Cos(float value) => (float)Math.Cos(value);
        public static float Acos(float value) => (float)Math.Acos(value);

        public static float Clamp(float value, float min, float max)
        {
            if (value < min)
                return min;
            if (value > max)
                return max;
            return value;
        }

        public static float Atan2(float y, float x) => (float)Math.Atan2(y, x);

        public static float Min(float v0, float v1) => v0 < v1 ? v0 : v1;
        public static float Max(float v0, float v1) => v0 > v1 ? v0 : v1;

        public static float Sqrt(float value) => (float)Math.Sqrt(value);

        public static float Abs(float value) => Math.Abs(value);
        public static int Abs(int value) => Math.Abs(value);


        public static Vector3 Abs(Vector3 value)
        {
            return new Vector3(Math.Abs(value.X), Math.Abs(value.Y), Math.Abs(value.Z));
        }

        public static float Lerp(float p1, float p2, float t)
        {
            if (t < 0.0f)
                return p1;
            else if (t > 1.0f)
                return p2;

            return p1 + ((p2 - p1) * t);
        }

        public static Vector3 Lerp(Vector3 p1, Vector3 p2, float t)
        {
            if (t < 0.0f)
                return p1;
            else if (t > 1.0f)
                return p2;

            return p1 + ((p2 - p1) * t);
        }

        // not the same as a%b
        public static float Modulo(float a, float b) => a - b * (float)Math.Floor(a / b);

        public static float Distance(float p1, float p2) => Abs(p1 - p2);
    }
}
