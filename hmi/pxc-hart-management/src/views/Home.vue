<template>
  <div class="container">
    <HartDevice longTag="Dummy Device" hartRev="7"/>
  </div>
</template>

<script>
// @ is an alias to /src
import HartDevice from '@/components/HartDevice.vue'

export default {
  name: 'home',
  components: {
    HartDevice
  }
}
</script>
