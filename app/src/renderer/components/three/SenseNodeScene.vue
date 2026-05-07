<script setup lang="ts">
import { computed, onBeforeUnmount, onMounted, ref, watch } from 'vue'
import * as THREE from 'three'
import { useI18n } from '../../lib/i18n'
import { useDeviceStore } from '../../stores/deviceStore'
import { useSerialStore } from '../../stores/serialStore'
import { useSettingsStore } from '../../stores/settingsStore'
import type { SenseNodeFrame } from '../../../shared/types/sensenode'

const canvasHost = ref<HTMLDivElement | null>(null)
const device = useDeviceStore()
const serial = useSerialStore()
const settings = useSettingsStore()
const { t } = useI18n()
const isConnected = computed(() => serial.status.state === 'connected')

let renderer: THREE.WebGLRenderer | null = null
let scene: THREE.Scene | null = null
let camera: THREE.PerspectiveCamera | null = null
let node: THREE.Group | null = null
let grid: THREE.GridHelper | null = null
let keyLight: THREE.DirectionalLight | null = null
let fillLight: THREE.AmbientLight | null = null
let boardMaterial: THREE.MeshStandardMaterial | null = null
let shellMaterial: THREE.LineBasicMaterial | null = null
let markerMaterials: THREE.MeshStandardMaterial[] = []
let resizeObserver: ResizeObserver | null = null
let animationId = 0
let targetRotation = new THREE.Euler(0, 0, 0)
const fixedCameraPosition = new THREE.Vector3(0.02, 1.12, 6.35)
const fixedCameraTarget = new THREE.Vector3(0.04, 0.1, 0)

const accelText = computed(() => formatVec3(device.currentFrame?.accel))
const gyroText = computed(() => formatVec3(device.currentFrame?.gyro))

onMounted(() => {
	if (!canvasHost.value) return
	scene = new THREE.Scene()
	camera = new THREE.PerspectiveCamera(32, 1, 0.1, 100)
	camera.position.copy(fixedCameraPosition)
	camera.lookAt(fixedCameraTarget)

	renderer = new THREE.WebGLRenderer({
		antialias: true,
		alpha: true,
		powerPreference: 'high-performance',
	})
	renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2))
	renderer.domElement.className = 'block h-full w-full'
	canvasHost.value.appendChild(renderer.domElement)

	keyLight = new THREE.DirectionalLight('#d6e4ff', 1.8)
	keyLight.position.set(2, 4, 3)
	fillLight = new THREE.AmbientLight('#9ba6b2', 1.2)
	scene.add(keyLight, fillLight)

	node = createNodeModel()
	scene.add(node)
	grid = createGrid()
	scene.add(grid)
	applySceneTheme(settings.themeMode)

	resizeObserver = new ResizeObserver(() => resize())
	resizeObserver.observe(canvasHost.value)
	window.addEventListener('resize', resize)
	resize()
	animate()
})

watch(
	() => device.currentFrame,
	(frame) => {
		targetRotation = resolveTargetRotation(frame)
	},
)

watch(
	() => settings.themeMode,
	(themeMode) => applySceneTheme(themeMode),
)

onBeforeUnmount(() => {
	resizeObserver?.disconnect()
	resizeObserver = null
	window.removeEventListener('resize', resize)
	cancelAnimationFrame(animationId)
	renderer?.dispose()
})

function animate(): void {
	animationId = requestAnimationFrame(animate)
	if (!renderer || !scene || !camera || !node) return

	node.rotation.x = THREE.MathUtils.lerp(
		node.rotation.x,
		targetRotation.x,
		0.24,
	)
	node.rotation.y = THREE.MathUtils.lerp(
		node.rotation.y,
		targetRotation.y,
		0.24,
	)
	node.rotation.z = THREE.MathUtils.lerp(
		node.rotation.z,
		targetRotation.z,
		0.24,
	)
	node.scale.setScalar(0.68)
	camera.position.copy(fixedCameraPosition)
	camera.lookAt(fixedCameraTarget)
	renderer.render(scene, camera)
}

function resize(): void {
	if (!canvasHost.value || !renderer || !camera) return
	const rect = canvasHost.value.getBoundingClientRect()
	renderer.setSize(rect.width, rect.height, false)
	camera.aspect = rect.width / rect.height
	camera.updateProjectionMatrix()
}

function createNodeModel(): THREE.Group {
	const group = new THREE.Group()
	boardMaterial = new THREE.MeshStandardMaterial({
		color: '#eff4fa',
		metalness: 0.04,
		roughness: 0.78,
		emissive: '#f4f7fb',
		emissiveIntensity: 0.08,
	})
	const board = new THREE.Mesh(
		new THREE.BoxGeometry(2.9, 0.22, 0.92),
		boardMaterial,
	)
	group.add(board)

	shellMaterial = new THREE.LineBasicMaterial({
		color: '#9fb0c4',
		transparent: true,
		opacity: 0.38,
	})
	const shell = new THREE.LineSegments(
		new THREE.EdgesGeometry(new THREE.BoxGeometry(3.02, 0.25, 1.04)),
		shellMaterial,
	)
	group.add(shell)

	const markerColors = ['#8c9db1', '#64748b', '#94a3b8', '#cbd5e1']
	for (let index = 0; index < 4; index += 1) {
		const material = new THREE.MeshStandardMaterial({
			color: markerColors[index],
			metalness: 0.08,
			roughness: 0.6,
		})
		markerMaterials.push(material)
		const marker = new THREE.Mesh(
			new THREE.CylinderGeometry(0.065, 0.065, 0.025, 24),
			material,
		)
		marker.rotation.x = Math.PI / 2
		marker.position.set(-1.02 + index * 0.68, 0.14, -0.24 + (index % 2) * 0.46)
		group.add(marker)
	}

	group.position.y = 0.04
	return group
}

function createGrid(): THREE.LineSegments {
	const helper = new THREE.GridHelper(7, 14, '#94a3b8', '#cbd5e1')
	helper.position.set(0, -0.5, 0)
	return helper
}

function resolveTargetRotation(frame: SenseNodeFrame | null): THREE.Euler {
	if (!frame?.accel) {
		return new THREE.Euler(0, 0, 0)
	}

	const gyroMag = vectorMag(frame.gyro)
	const accelDelta = Math.abs(vectorMag(frame.accel) - 1)
	if (gyroMag < 0.55 && accelDelta < 0.24) {
		return new THREE.Euler(0, 0, 0)
	}

	const pitch = -(frame.pitch ?? Math.atan2(frame.accel.y, frame.accel.z))
	const roll = -(
		frame.roll ??
		Math.atan2(
			-frame.accel.x,
			Math.sqrt(frame.accel.y ** 2 + frame.accel.z ** 2),
		)
	)
	const yaw = frame.yaw ?? targetRotation.y * 0.96
	return new THREE.Euler(clampAngle(roll), clampAngle(yaw), clampAngle(pitch))
}

function vectorMag(value?: { x: number; y: number; z: number }): number {
	if (!value) {
		return 0
	}
	return Math.sqrt(value.x * value.x + value.y * value.y + value.z * value.z)
}

function formatVec3(value?: { x: number; y: number; z: number }): string {
	if (serial.status.state !== 'connected' || !value) {
		return '- / - / -'
	}

	return [value.x, value.y, value.z]
		.map((component) => component.toFixed(2))
		.join(' / ')
}

function clampAngle(value: number): number {
	return THREE.MathUtils.clamp(value, -Math.PI * 0.7, Math.PI * 0.7)
}

function applySceneTheme(themeMode: 'dark' | 'light'): void {
	if (!scene || !keyLight || !fillLight || !grid) return

	if (themeMode === 'light') {
		scene.background = new THREE.Color('#f8fafc')
		keyLight.color.set('#1f2937')
		keyLight.intensity = 1.55
		fillLight.color.set('#94a3b8')
		fillLight.intensity = 1.55
		setGridColors(grid, '#aab7c8', '#d7e1ed')
		setBoardTheme('#eff4fa', '#f4f7fb', 0.04, 0.78, '#8c9db1', 0.38)
		return
	}

	scene.background = new THREE.Color('#080A0F')
	keyLight.color.set('#d6e4ff')
	keyLight.intensity = 1.8
	fillLight.color.set('#9ba6b2')
	fillLight.intensity = 1.2
	setGridColors(grid, '#1f2937', '#111827')
	setBoardTheme('#17251f', '#0d1b16', 0.2, 0.55, '#b4c7b4', 0.36)
}

function setGridColors(
	helper: THREE.GridHelper,
	center: string,
	gridColor: string,
): void {
	const material = helper.material
	if (Array.isArray(material)) {
		material[0]?.color.set(center)
		material[1]?.color.set(gridColor)
	}
}

function setBoardTheme(
	boardColor: string,
	boardEmissive: string,
	metalness: number,
	roughness: number,
	markerColor: string,
	lineOpacity: number,
): void {
	if (boardMaterial) {
		boardMaterial.color.set(boardColor)
		boardMaterial.emissive.set(boardEmissive)
		boardMaterial.metalness = metalness
		boardMaterial.roughness = roughness
	}

	if (shellMaterial) {
		shellMaterial.opacity = lineOpacity
		shellMaterial.color.set(markerColor)
	}

	for (const material of markerMaterials) {
		material.color.set(markerColor)
		material.metalness = 0.06
		material.roughness = 0.58
	}
}
</script>

<template>
	<div class="relative h-full w-full overflow-hidden">
		<div ref="canvasHost" class="h-full w-full" />
		<div
			class="pointer-events-none absolute bottom-24 left-1/2 -translate-x-1/2"
		>
			<div
				class="w-max max-w-[calc(100vw-2rem)] overflow-hidden text-xs text-muted"
				:class="isConnected ? 'ag-visible' : 'ag-hidden'"
			>
				<div
					class="inline-flex flex-nowrap items-start justify-center gap-4 whitespace-nowrap"
				>
					<div class="grid shrink-0 grid-cols-[auto_auto] gap-2">
						<span class="font-medium text-highlighted">A</span>
						<span class="tabular-nums">{{ accelText }}</span>
					</div>
					<div class="grid shrink-0 grid-cols-[auto_auto] gap-2">
						<span class="font-medium text-highlighted">G</span>
						<span class="tabular-nums">{{ gyroText }}</span>
					</div>
				</div>
			</div>
		</div>
	</div>
</template>
