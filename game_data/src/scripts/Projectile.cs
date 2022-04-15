namespace Dc
{
    public class Projectile : Entity
    {
        public void OnCreate()
        {
            TriggerBeginEvent += OnTriggerBegin;
        }

        public void OnTriggerBegin(Entity other)
        {
            Log.Info("Projectile trigger");
            Scene.RemoveEntity(other);
            Scene.RemoveEntity(this);
        }
    }
}
