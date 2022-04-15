namespace Dc
{
    class Enemy : Entity
    {
        private float speed = 5.0f;

        public void OnUpdate(float deltaTime)
        {
            Position += Vector3.Back * speed * deltaTime;
        }
    }
}
