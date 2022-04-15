namespace Dc
{
    class Player : Entity
    {
        private float PlayerSpeed = 10.0f;
        private float ProjectileSpeed = 10.0f;

        public void OnUpdate(float deltaTime)
        {
            if (Input.IsKeyPressed(Key.A))
            {
                Position += Vector3.Left * PlayerSpeed * deltaTime;
            }
            if (Input.IsKeyPressed(Key.D))
            {
                Position += Vector3.Right * PlayerSpeed * deltaTime;
            }
            if (Input.IsKeyPressed(Key.Space))
            {
                // Create projectile entity
                var projectile = Scene.CreateEntity("Projectile");
                projectile.Position = Position + new Vector3(0.0f, 2.0f, 0.0f);

                var mesh = projectile.CreateComponent<MeshComponent>();
                mesh.SetMesh("meshes/projectile.dcmesh");

                var sphere = projectile.CreateComponent<SphereColliderComponent>();
                sphere.IsTrigger = true;

                var rigidBody = projectile.CreateComponent<RigidBodyComponent>();
                // rigidBody.IsKinematic = false;
                rigidBody.IsGravityDisabled = true;
                rigidBody.LinearVelocity = Vector3.Forward * ProjectileSpeed;

                var script = projectile.CreateComponent<ScriptComponent>();
                script.SetScript("Dc.Projectile");
            }
        }
    }
}
