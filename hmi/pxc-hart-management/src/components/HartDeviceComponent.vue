<template>
<div class="hart-device">
    <div class="row">
        <b-alert class="mb-3" show variant="warning" v-for="status in device.extendedDeviceStatus" v-bind:key="status.name">
            <b>{{ status.name }}</b> {{ status.description }}
        </b-alert>
    </div>
    <h1 class="text-center">{{ device.longTag }}</h1>
    <b-card-group deck>
        <b-card header="Basic Info" class="text-center text-dark">
            <table>
                <tr v-for="(attr, key) in hartAttrs" v-bind:key="key">
                    <th style="text-align: right; padding-right: 0.5em;">{{ attr }}</th>
                    <td style="text-align: left; padding-right: 0.5em;" class="text-primary">{{ device[key] }}</td>
                </tr>
            </table>
        </b-card>
        <HartVarChart :log-data="logData.pv" title="PV"></HartVarChart>
    </b-card-group>

    <b-card-group deck class="mt-3">
        <HartVarChart :log-data="logData.sv" title="SV" size="small"></HartVarChart>
        <HartVarChart :log-data="logData.tv" title="TV" size="small"></HartVarChart>
        <HartVarChart :log-data="logData.qv" title="QV" size="small"></HartVarChart>
    </b-card-group>

    <section id="send-cmd" class="mt-3">
        <div class="input-group">
            <!-- <input type="text" class="form-control" placeholder="command #" aria-label="cmdnum" aria-describedby="basic-addon2"> -->
            <b-dropdown id="dropdown-1" :text="'Send a Command'" class="m-md-2" variant="primary" size="lg">
                <b-dropdown-item v-for="cmd in supportedCommands" v-bind:key="cmd.number" v-on:click="sendCmd(cmd);">
                    {{ cmd.number + ": " + cmd.description }}</b-dropdown-item>
            </b-dropdown>
            <!-- <div class="input-group-append">
                <button class="btn btn-outline-secondary" type="button" v-on:click="sendCmd()">Send</button>
            </div> -->
        </div>
        <div id="cmd-result mt-3">
            <h3 v-if="activeCommand != null">Command {{ activeCommand.number }} ({{ activeCommand.description }})</h3>
            <b-card no-body v-if="cmdResponse != null">
                <b-tabs card>
                <b-tab title="Table" active>
                    <b-table striped hover :items="cmdResponse.data"></b-table>
                </b-tab>
                <b-tab title="JSON">
                    <vue-json-pretty
                        :path="'res'"
                        :data="cmdResponse">
                    </vue-json-pretty>
                </b-tab>
                </b-tabs>
            </b-card>
            <b-spinner v-if="activeCommand != null && cmdResponse == null"
                variant="primary"
            ></b-spinner>
        </div>
    </section>
</div>
</template>

<script lang="ts">
import { Vue, Component, Prop } from 'vue-property-decorator'
import { HartDeviceDto, verboseHartDeviceAttrs, hart_command_brief, hart_command_response, hart_device_vars_log } from '../types/hart-device'
import { HartVars } from '../types/hart-vars'
import { hartServerUrl, updateInterval } from '../types'
// @ts-ignore
import VueJsonPretty from 'vue-json-pretty'
import HartVarChart from '../components/HartVarChart.vue'
import { ChartData } from 'chart.js'

@Component({
    components: {
        VueJsonPretty,
        HartVarChart
    }
})
export default class HartDeviceComponent extends Vue {
    @Prop() device!: HartDeviceDto
    public supportedCommands: hart_command_brief[] = []
    public activeCommand: hart_command_brief | null = null
    public cmdResponse: hart_command_response | null = null
    public hartAttrs = verboseHartDeviceAttrs
    public logData: hart_device_vars_log = {
        pv: [],
        sv: [],
        tv: [],
        qv: []
    }
    private logPollInterval: number = 0

    mounted() {
        setTimeout(this.update, 500) // wait for things to load
        this.logPollInterval = setInterval(this.update, updateInterval)
    }

    beforeDestroy () {
        clearInterval(this.logPollInterval)
    }

    get vars(): HartVars | undefined {
        return this.device.vars
    }

    sendCmd(cmd: hart_command_brief) {
        this.activeCommand = cmd;
        this.cmdResponse = null;
        this.$http.get(hartServerUrl + '/gw/' + this.$route.params.serialNo + '/subdevice/' + this.device.ioCard + '/' + this.device.channel + '/cmd/' + cmd.number).then(res => {
            const data = res.data as hart_command_response
            this.cmdResponse = data
        })
    }

    private update() {
        this.refreshChartsData();
        if (this.supportedCommands.length == 0) {
            // load supported HART commands
            this.$http.get(hartServerUrl + '/gw/' + this.$route.params.serialNo + '/subdevice/' + this.device.ioCard + '/' + this.device.channel + '/commands').then(res => {
                const data = res.data as hart_command_brief[]
                this.supportedCommands = data
            })
        }
    }

    private refreshChartsData() {
        this.$http.get(hartServerUrl + '/gw/' + this.$route.params.serialNo + '/log/' + this.device.ioCard + '/' + this.device.channel).then(res => {
            this.logData = res.data as hart_device_vars_log
        })
    }
}
</script>

<style scoped>
h1 {
    margin: 0 auto 2rem auto;
}
</style>