using System;
using System.Runtime.InteropServices;

namespace Dc
{
    [StructLayout(LayoutKind.Explicit)]
    public struct Matrix4
    {
        [FieldOffset(0)] public float D00;
        [FieldOffset(4)] public float D10;
        [FieldOffset(8)] public float D20;
        [FieldOffset(12)] public float D30;
        [FieldOffset(16)] public float D01;
        [FieldOffset(20)] public float D11;
        [FieldOffset(24)] public float D21;
        [FieldOffset(28)] public float D31;
        [FieldOffset(32)] public float D02;
        [FieldOffset(36)] public float D12;
        [FieldOffset(40)] public float D22;
        [FieldOffset(44)] public float D32;
        [FieldOffset(48)] public float D03;
        [FieldOffset(52)] public float D13;
        [FieldOffset(56)] public float D23;
        [FieldOffset(60)] public float D33;

        public Matrix4(float value)
        {
            D00 = value; D10 = 0.0f; D20 = 0.0f; D30 = 0.0f;
            D01 = 0.0f; D11 = value; D21 = 0.0f; D31 = 0.0f;
            D02 = 0.0f; D12 = 0.0f; D22 = value; D32 = 0.0f;
            D03 = 0.0f; D13 = 0.0f; D23 = 0.0f; D33 = value;
        }

        public Vector3 Translation
        {
            get { return new Vector3(D03, D13, D23); }
            set { D03 = value.X; D13 = value.Y; D23 = value.Z; }
        }

        public static Matrix4 Translate(Vector3 translation)
        {
            return new Matrix4(1.0f)
            {
                D03 = translation.X,
                D13 = translation.Y,
                D23 = translation.Z
            };
        }

        public static Matrix4 Scale(Vector3 scale)
        {
            return new Matrix4(1.0f)
            {
                D00 = scale.X,
                D11 = scale.Y,
                D22 = scale.Z
            };
        }

        public static Matrix4 Scale(float scale)
        {
            return new Matrix4(1.0f)
            {
                D00 = scale,
                D11 = scale,
                D22 = scale
            };
        }

        public void DebugPrint()
        {
            Console.WriteLine("{0:0.00}  {1:0.00}  {2:0.00}  {3:0.00}", D00, D10, D20, D30);
            Console.WriteLine("{0:0.00}  {1:0.00}  {2:0.00}  {3:0.00}", D01, D11, D21, D31);
            Console.WriteLine("{0:0.00}  {1:0.00}  {2:0.00}  {3:0.00}", D02, D12, D22, D32);
            Console.WriteLine("{0:0.00}  {1:0.00}  {2:0.00}  {3:0.00}", D03, D13, D23, D33);
        }

    }
}
