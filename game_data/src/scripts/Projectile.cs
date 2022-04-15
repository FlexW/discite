namespace Dc
{
    public class Projectile : Entity
    {
        public void OnCreate()
        {
            TriggerBeginEvent += OnTriggerBegin;
        }

        public void OnUpdate(float deltaTime)
        {
            if (Position.Z < -60.0f)
            {
                Scene.RemoveEntity(this);
            }
        }

        public void OnTriggerBegin(Entity other)
        {
            Log.Info("Projectile trigger");
            Scene.RemoveEntity(other);
            Scene.RemoveEntity(this);
        }
    }
}
