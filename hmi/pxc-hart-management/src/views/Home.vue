<template>
  <div class="container">
    <main role="main" class="text-center">
      <h1 class="cover-heading">Phoenix Contact HART Management System</h1>
      <p class="lead">Easily view and manage your HART device network.</p>
      <p class="lead">
        Detected Hart Gateways:
        <HartGateways />
      </p>
    </main>
  </div>
</template>

<script>
// @ is an alias to /src
import HartGateways from '@/components/HartGateways.vue'

export default {
  name: 'home',
  components: {
    HartGateways
  }
}
</script>

<style lang="scss" scoped>

main {
  max-width: 800px;
  margin: 2em auto;
}
</style>