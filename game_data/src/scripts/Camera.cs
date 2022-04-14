namespace Dc
{
    class Camera : Entity
    {
        private float Speed = 20.0f;

        void OnUpdate(float deltaTime)
        {
            if (Input.IsKeyPressed(Key.W))
            {
                Position += new Vector3(0.0f, 0.0f, -1.0f) * Speed * deltaTime;
            }

            if (Input.IsKeyPressed(Key.S))
            {
                Position += new Vector3(0.0f, 0.0f, 1.0f) * Speed * deltaTime;
            }

            if (Input.IsKeyPressed(Key.A)) { }

            if (Input.IsKeyPressed(Key.D)) { }
        }
    }
}
